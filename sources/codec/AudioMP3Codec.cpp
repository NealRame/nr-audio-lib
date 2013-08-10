/*
 * AudioMP3Codec.cpp
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

extern "C" {
#	include <byteswap.h>
#	include <lame/lame.h>
}

#include <cstring>
#include <cstdint>
#include <iostream>

#include <boost/format.hpp>
#include <boost/detail/endian.hpp>

#include "../AudioError.h"
#include "../AudioBuffer.h"
#include "../AudioFormat.h"

#include "AudioMP3Coder.h"
#include "AudioMP3Decoder.h"

namespace com {
namespace nealrame {
namespace audio {

u_int32_t synchsafe(u_int32_t in) {
#if defined (BOOST_LITTLE_ENDIAN)
	in = bswap_32(in);
#endif
	u_int32_t out, mask = 0x7F;
	while (mask ^ 0x7FFFFFFF) {
		out = in & ~mask;
		out <<= 1;
		out |= in & mask;
		mask = ((mask + 1) << 8) - 1;
		in = out;
	}
	return out;
}

u_int32_t unsynchsafe(u_int32_t in) {
#if defined (BOOST_LITTLE_ENDIAN)
	in = bswap_32(in);
#endif
	u_int32_t out = 0, mask = 0x7F000000;
	while (mask) {
		out >>= 1;
		out |= in & mask;
		mask >>= 8;
	}
	return out;
}

struct ID3V2Header {
	char tag[3];
	u_int8_t version_major;
	u_int8_t version_minor;
	u_int8_t flags;
	u_int32_t size;
} __attribute__((packed));

bool check_id3_tag_word(u_int32_t word) {
	return memcmp((void *)&word, "ID3", 3) == 0;
}

void skip_id3_sections(std::ifstream &input) {
	u_int32_t dword;
	bool stop = false;

	do {
		std::streampos pos = input.tellg();

		input.read((char *)&dword, sizeof(dword));

		if (input.fail() || (uint)input.gcount() < sizeof(dword)) {
			Error::raise(Error::Status::IOError);
		}

		input.seekg(pos);

		if (check_id3_tag_word(dword)) {
			ID3V2Header id3v2_header;
			input.read((char *)&id3v2_header, sizeof(id3v2_header));
			if (input.fail() || (uint)input.gcount() < sizeof(id3v2_header)) {
				Error::raise(Error::Status::IOError);
			}
			id3v2_header.size = unsynchsafe(id3v2_header.size);
			input.seekg(id3v2_header.size, std::ifstream::cur);
		} else {
			stop = true;
		}
	} while (! stop);
}

bool check_album_id_word(u_int32_t word) {
	return memcmp((void *)&word, "AiD\1", sizeof(word)) == 0;
}

void skip_album_id_section(std::ifstream &input) {
	u_int32_t dword;
	bool stop = false;

	do {
		std::streampos pos = input.tellg();

		input.read((char *)&dword, sizeof(dword));
		if (input.fail() || (uint)input.gcount() < sizeof(dword)) {
			Error::raise(Error::Status::IOError);
		}

		if (check_album_id_word(dword)) {
			u_int16_t section_size;
			input.read((char *)&section_size, sizeof(section_size));
			if (input.fail() || (uint) input.gcount() < sizeof(section_size)) {
				Error::raise(Error::Status::IOError);
			}
#if ! defined (BOOST_LITTLE_ENDIAN)
			section_size = bswap_16(section_size);
#endif
			input.seekg(section_size - 6, std::ifstream::cur);
		} else {
			input.seekg(pos);
			stop = true;
		}
	} while (! stop);
}

struct RAIIDecodeData {
	hip_global_flags *hip;
	mp3data_struct format;
	int16_t *pcm_buffer[2];
	int enc_delay;
	int enc_padding;
	Buffer *audio_buffer;

	RAIIDecodeData( ) {
		if ((hip = hip_decode_init()) == nullptr) {
			Error::raise(Error::Status::LameInitializationFailed);
		}
		memset(&format, 0, sizeof(format));
		pcm_buffer[0] = new int16_t[1152];
		pcm_buffer[1] = new int16_t[1152];
		enc_delay = enc_padding = 0;
		audio_buffer = nullptr;
	}

	virtual ~RAIIDecodeData( ) {
		if (audio_buffer != nullptr) delete audio_buffer;
		if (hip != nullptr) hip_decode_exit(hip);
		delete pcm_buffer[0];
		delete pcm_buffer[1];
	}
};

#if defined DEBUG
#	define DEBUG_MP3_FORMAT_HEADER(FMT) \
do { \
	std::cerr << "total frames  : " << (FMT).totalframes << std::endl; \
	std::cerr << "channel count : " << (FMT).stereo << std::endl; \
	std::cerr << "sample rate   : " << (FMT).samplerate << std::endl; \
	std::cerr << "sample count  : " << (FMT).nsamp << std::endl; \
} while (0)
#else
#	define DEBUG_MP3_FORMAT_HEADER(...)
#endif

Buffer * MP3Decoder::decode(std::ifstream &input) const {
	skip_id3_sections(input);
	skip_album_id_section(input);

	RAIIDecodeData decode_data;

	u_int8_t buffer[512];
	int len, offset = 0, ret = 0;

	while (decode_data.format.header_parsed == 0) {
		input.read((char *)buffer, sizeof(buffer));
		if (input.fail()) {
			Error::raise(Error::Status::IOError);
		}

		len = input.gcount();

		if ((ret = hip_decode1_headersB(
				decode_data.hip, buffer, len,
				decode_data.pcm_buffer[0], decode_data.pcm_buffer[1],
				&decode_data.format, &decode_data.enc_delay, &decode_data.enc_padding)) < 0) {
			Error::raise(Error::Status::LameDecodingFailed);
		}
	}

	if (decode_data.format.bitrate == 0) {
		Error::raise(Error::Status::LameDecodingFailed);
	}

	DEBUG_MP3_FORMAT_HEADER(decode_data.format);

	decode_data.audio_buffer =
		new Buffer(
			Format{(unsigned int)decode_data.format.stereo,
			(Format::SampleRate)decode_data.format.samplerate,
			Format::BitDepth_16});

	len = 0;
	if ((ret = hip_decode1_headers(
			decode_data.hip, buffer, len,
			decode_data.pcm_buffer[0], decode_data.pcm_buffer[1],
			&decode_data.format)) < 0) {
		Error::raise(Error::Status::LameDecodingFailed);
	}

	decode_data.audio_buffer->write(offset, ret, (const int16_t **)decode_data.pcm_buffer);
	offset += ret;

	do {
		input.read((char *)buffer, sizeof(buffer));
		if (input.bad()) {
			Error::raise(Error::Status::IOError);
		}
		len = input.gcount();

		if ((ret = hip_decode1_headers(
				decode_data.hip, buffer, len,
				decode_data.pcm_buffer[0], decode_data.pcm_buffer[1],
				&decode_data.format)) < 0) {
			Error::raise(Error::Status::LameDecodingFailed);
		}

		decode_data.audio_buffer->write(offset, ret, (const int16_t **)decode_data.pcm_buffer);
		offset += ret;
	} while (len > 0);

	Buffer *audio_buffer = decode_data.audio_buffer;
	decode_data.audio_buffer = nullptr;

	return audio_buffer;
}

void MP3Coder::encode(const Buffer &, std::ofstream &) const {
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
