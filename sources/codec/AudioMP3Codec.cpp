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
#include <sstream>

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

//////////////////////////////////////////////////////////////////////////////
// Decoder ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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

struct RAII_MP3DecoderData {
	std::ifstream &input;
	std::ifstream::iostate input_state;
	hip_global_flags *hip;
	mp3data_struct format;
	int16_t *pcm_buffer[2];
	int enc_delay;
	int enc_padding;
	Buffer *audio_buffer;

	RAII_MP3DecoderData(std::ifstream &in) :
		input(in) {

		input_state = input.exceptions();
		input.exceptions(std::ifstream::badbit);

		if ((hip = hip_decode_init()) == nullptr) {
			Error::raise(Error::Status::MP3CodecError, "Failed to init lame encoder.");
		}

		memset(&format, 0, sizeof(format));

		pcm_buffer[0] = new int16_t[1152];
		pcm_buffer[1] = new int16_t[1152];
		enc_delay = enc_padding = 0;

		audio_buffer = nullptr;
	}

	virtual ~RAII_MP3DecoderData() {
		input.exceptions(input_state);
		if (audio_buffer != nullptr) delete audio_buffer;
		if (hip != nullptr) hip_decode_exit(hip);
		delete pcm_buffer[0];
		delete pcm_buffer[1];
	}
};

#if defined(DEBUG)
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
	RAII_MP3DecoderData decode_data(input);

	skip_id3_sections(input);
	skip_album_id_section(input);

	uint8_t buffer[512];
	int len, offset = 0, ret = 0;

	while (decode_data.format.header_parsed == 0) {
		input.read((char *)buffer, sizeof(buffer));
		len = input.gcount();

		if (len == 0) {
			Error::raise(Error::Status::IOError, "The file is truncated.");
		}

		if ((ret = hip_decode1_headersB(
				decode_data.hip, buffer, len,
				decode_data.pcm_buffer[0], 
				decode_data.pcm_buffer[1],
				&decode_data.format, 
				&decode_data.enc_delay, 
				&decode_data.enc_padding)) < 0) {
			Error::raise(Error::Status::MP3CodecError);
		}
	}

	if (decode_data.format.bitrate == 0) {
		Error::raise(Error::Status::MP3CodecError);
	}

	DEBUG_MP3_FORMAT_HEADER(decode_data.format);

	decode_data.audio_buffer =
		new Buffer(
			Format((unsigned int)decode_data.format.stereo, 
				(unsigned int)decode_data.format.samplerate, 
				16));

	len = 0;
	if ((ret = hip_decode1_headers(
			decode_data.hip, buffer, len,
			decode_data.pcm_buffer[0], decode_data.pcm_buffer[1],
			&decode_data.format)) < 0) {
		Error::raise(Error::Status::MP3CodecError);
	}

	decode_data.audio_buffer->write(offset, ret, (const int16_t **)decode_data.pcm_buffer);
	offset += ret;

	do {
		input.read((char *)buffer, sizeof(buffer));
		len = input.gcount();

		int pos = input.tellg();

		if (pos > 0) {
			std::cerr << pos << std::endl;
		}

		if ((ret = hip_decode1_headers(
				decode_data.hip, buffer, len,
				decode_data.pcm_buffer[0], decode_data.pcm_buffer[1],
				&decode_data.format)) < 0) {
			Error::raise(Error::Status::MP3CodecError);
		}

		decode_data.audio_buffer->write(offset, ret, (const int16_t **)decode_data.pcm_buffer);
		offset += ret;
	} while (len > 0);

	Buffer *audio_buffer = decode_data.audio_buffer;
	decode_data.audio_buffer = nullptr;

	return audio_buffer;
}

//////////////////////////////////////////////////////////////////////////////
// Coder /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int lame_quality(Coder::Quality quality) {
	switch (quality) {
	case Coder::Quality::Best: return 0;
	case Coder::Quality::Good: return 2;
	case Coder::Quality::Acceptable: return 5;
	case Coder::Quality::Fastest: return 7;
	}
	return 0;
}

#define ERR_HANDLER_DEFAULT_BUFFER_SIZE 128
#define MP3_ENCODE_INPUT_BUFFER_SIZE   1024

void error_handler(const char *fmt, va_list ap) {
	char **buffer = nullptr;
	size_t buffer_size = ERR_HANDLER_DEFAULT_BUFFER_SIZE;
	bool again;
	do {
		size_t written;
		*buffer = (char *)realloc(*buffer, buffer_size);

		written = vsnprintf(*buffer, buffer_size, fmt, ap);

		if (written >= buffer_size) {
			again = true;
			buffer_size = written;
		} else {
			again = false;
		}
	} while (again);
	Error::raise(Error::Status::MP3CodecError, std::string(*buffer));
}

#if defined(DEBUG)
void debug_handler(const char *, va_list ap) {
	fvprintf(stderr, "%s\n", ap);
}

void message_handler(const char *, va_list) {
	fvprintf(stderr, "%s\n", ap);
}
#else
void debug_handler(const char *, va_list) {}
void message_handler(const char *, va_list) {}
#endif

struct RAII_MP3CoderData {
	std::ofstream &output;
	std::ofstream::iostate output_state;
	lame_t gfp;
	int mp3_output_buffer_size, mp3_input_buffer_size;
	char *mp3_output_buffer;
	float *mp3_input_buffer;

	RAII_MP3CoderData(const MP3Coder &coder, const Buffer &buffer, std::ofstream &out) :
		output(out) {

		output_state = output.exceptions();
		output.exceptions(std::ofstream::failbit | std::ofstream::badbit );

		if ((gfp = lame_init()) == nullptr) {
			Error::raise(Error::Status::MP3CodecError, "Failed to init lame encoder.");
		}

		lame_set_errorf(gfp, error_handler);
		lame_set_debugf(gfp, debug_handler);
		lame_set_msgf  (gfp, message_handler);

		Format format = buffer.format();

		mp3_output_buffer_size = 1.25*buffer.frameCount() + 7200;
		mp3_output_buffer = 
			new char[mp3_output_buffer_size];

		mp3_input_buffer_size = format.channelCount()*MP3_ENCODE_INPUT_BUFFER_SIZE;
		mp3_input_buffer = 
			new float[mp3_input_buffer_size];

		lame_set_num_channels(gfp, format.channelCount());
		lame_set_in_samplerate(gfp, format.sampleRate());
		lame_set_quality(gfp, lame_quality(coder.quality()));
		lame_set_bWriteVbrTag(gfp, 0);

		lame_init_params(gfp);
	}

	virtual ~RAII_MP3CoderData( ) {
		if (gfp != nullptr) {
			lame_close(gfp);
			delete[] mp3_output_buffer;
			delete[] mp3_input_buffer;
		}
		output.exceptions(output_state);
	}
};

void MP3Coder::encode(const Buffer &buffer, std::ofstream &out) const {
	RAII_MP3CoderData encode_data(*this, buffer, out);

	unsigned int offset = 0;
	int nbytes;

	do {
		unsigned int count = buffer.read(offset, MP3_ENCODE_INPUT_BUFFER_SIZE, encode_data.mp3_input_buffer);

		nbytes = lame_encode_buffer_interleaved_ieee_float(
				encode_data.gfp, 
				encode_data.mp3_input_buffer, 
				count,
				(unsigned char *)encode_data.mp3_output_buffer,
				encode_data.mp3_output_buffer_size);

		if (nbytes >= 0) {
			out.write((char *)encode_data.mp3_output_buffer, nbytes);
		} else {
			Error::raise(Error::Status::MP3CodecError,
				(boost::format("Lame encode error code: %1%") % nbytes).str());
		}

		offset += count;
	} while (offset < buffer.frameCount());

	nbytes = lame_encode_flush(encode_data.gfp,
				(unsigned char *)encode_data.mp3_output_buffer,
				encode_data.mp3_output_buffer_size);

	if (nbytes >= 0) {
		out.write((char *)encode_data.mp3_output_buffer, nbytes);
	}
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
