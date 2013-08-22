/*
 * AudioMP3Codec.cpp
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

extern "C" {
#	include <byteswap.h>
#       include <ogg/ogg.h>
#       include <vorbis/codec.h>
#	include <vorbis/vorbisenc.h>
}

#include <cstring>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <sstream>

#include <boost/format.hpp>
#include <boost/detail/endian.hpp>

#include "../AudioError.h"
#include "../AudioBuffer.h"
#include "../AudioFormat.h"

#include "AudioOggVorbisCoder.h"
#include "AudioOggVorbisDecoder.h"

#define DEBUG_OGG_PACKET(OP) \
do { \
	fprintf(stderr, "op.bytes:      %li\n",  (OP).bytes); \
	fprintf(stderr, "op.b_o_s:      %li\n",  (OP).b_o_s); \
	fprintf(stderr, "op.e_o_s:      %li\n",  (OP).e_o_s); \
	fprintf(stderr, "op.granulepos: %lli\n", (OP).granulepos); \
	fprintf(stderr, "op.packetno:   %lli\n", (OP).packetno); \
	for (int i=0; i<(OP).bytes; ++i) { \
		if (! (i % 16)) { \
			fprintf(stderr, "%06x | %02x", i, (OP).packet[i]); \
		} else { \
			fprintf(stderr, " %02x", (OP).packet[i]); \
		} \
		if ((i % 16) == 15 || i == ((OP).bytes - 1)) { \
			fprintf(stderr, "\n"); \
		} \
	} \
} while (0)

namespace com {
namespace nealrame {
namespace audio {

std::string vorbis_error_string(int ov_code) {
	switch (ov_code) {
	case OV_FALSE:
		return "Not true, or no data available";

	case OV_HOLE:
		return "Vorbisfile encoutered missing or corrupt data in the bitstream.";

	case OV_EREAD:
		return "Read error while fetching compressed data for decode";

	case OV_EFAULT:
		return "Internal inconsistency in encode or decode state. Continuing is likely not possible";

	case OV_EIMPL:
		return "Feature not implemented";

	case OV_EINVAL:
		return "Either an invalid argument, or incompletely initialized argument passed to a call";

	case OV_ENOTVORBIS:
		return "The given file/data was not recognized as Ogg Vorbis data";

	case OV_EBADHEADER:
		return "The file/data is apparently an Ogg Vorbis stream, but contains a corrupted or undecipherable header";

	case OV_EVERSION:
		return "The bitstream format revision of the given stream is not supported";

	case OV_EBADLINK:
		return "The given link exists in the Vorbis data stream, but is not decipherable due to garbage or corruption";

	case OV_ENOSEEK:
		return "The given stream is not seekable";

	default:
		return "";
	}
}

float vorbis_quality(Coder::Quality quality) {
	switch (quality) {
	case Coder::Quality::Best: return 1.0;
	case Coder::Quality::Good: return 0.7;
	case Coder::Quality::Acceptable: return 0.3;
	case Coder::Quality::Fastest: return -0.1;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Decoder ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct RAII_OggDecodeData;
void decode_ogg_page_out(RAII_OggDecodeData &, ogg_page &);
void decode_ogg_packet_out(RAII_OggDecodeData &, ogg_packet &);

struct RAII_OggDecodeData {
	std::ifstream &input;
	std::ifstream::iostate input_state;

	ogg_sync_state o_sync;
	ogg_stream_state o_state;

	RAII_OggDecodeData(std::ifstream &in) :
		input(in) {
		input_state = input.exceptions();
		input.exceptions(std::ifstream::badbit);

		ogg_sync_init(&o_sync);

		ogg_page page;
		decode_ogg_page_out(*this, page);

		int stream_serial = ogg_page_serialno(&page);

		if (ogg_stream_init(&o_state, stream_serial) < 0) {
			Error::raise(Error::Status::OggVorbisError, "Ogg internal error.");
		}

		if (ogg_stream_pagein(&o_state, &page) < 0) {
			Error::raise(Error::Status::OggVorbisError, "Ogg internal error.");
		}
	}

	virtual ~RAII_OggDecodeData( ) {
		input.exceptions(input_state);
		ogg_sync_clear(&o_sync);
		ogg_stream_clear(&o_state);
	}
};

struct RAII_VorbisDecodeData {
	vorbis_info v_state;
	vorbis_comment v_comment;
	vorbis_dsp_state v_dsp;
	vorbis_block v_block;
	Buffer *buffer;

	RAII_VorbisDecodeData(RAII_OggDecodeData &ogg_decode_data) {
		int status;

		buffer = nullptr;

		vorbis_info_init(&v_state);
		vorbis_comment_init(&v_comment);

		ogg_packet packet;

		decode_ogg_packet_out(ogg_decode_data, packet);
		if ((status = vorbis_synthesis_headerin(&v_state, &v_comment, &packet)) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}

		decode_ogg_packet_out(ogg_decode_data, packet);
		if ((status = vorbis_synthesis_headerin(&v_state, &v_comment, &packet)) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}

		decode_ogg_packet_out(ogg_decode_data, packet);
		if ((status = vorbis_synthesis_headerin(&v_state, &v_comment, &packet)) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}

		if (vorbis_synthesis_init(&v_dsp, &v_state) != 0) {
			Error::raise(Error::Status::OggVorbisError, "Vorbis internal error.");
		}

		if (vorbis_block_init(&v_dsp, &v_block) != 0) {
			Error::raise(Error::Status::OggVorbisError, "Vorbis internal error.");
		}

		buffer = new Buffer(Format(v_state.channels, v_state.rate, 16));
	}

	virtual ~RAII_VorbisDecodeData() {
		if (buffer != nullptr) delete buffer;
		vorbis_block_clear(&v_block);
		vorbis_dsp_clear(&v_dsp);
		vorbis_info_clear(&v_state);
		vorbis_comment_clear(&v_comment);
	}
};

void decode_ogg_page_out(RAII_OggDecodeData &decode_data, ogg_page &page) {
	std::ifstream &input = decode_data.input;
	ogg_sync_state *o_sync =  &decode_data.o_sync;

	char *buffer;
	std::streamsize bytes;
	int status;

	while ((status = ogg_sync_pageout(&decode_data.o_sync, &page)) == 0) { // need more data
		buffer = ogg_sync_buffer(&decode_data.o_sync, 8192);
		input.read(buffer, 8192);

		bytes = input.gcount();

		if (! (bytes > 0 && ogg_sync_wrote(o_sync, bytes) == 0)) {
			status = -1;
			break;
		}
	}

	if (status < 0) {
		Error::raise(Error::Status::OggVorbisError, "Failed to read an Ogg page.");
	}
}

void decode_ogg_packet_out(RAII_OggDecodeData &decode_data, ogg_packet &packet) {
	ogg_sync_state *o_sync =  &decode_data.o_sync;

	if (ogg_sync_check(o_sync) != 0) {
		Error::raise(Error::Status::OggVorbisError, "Failed to read Ogg packet.");
	}

	ogg_stream_state *o_state = &decode_data.o_state;

	if (ogg_stream_packetout(o_state, &packet) != 1) {
		ogg_page page;
		decode_ogg_page_out(decode_data, page);
		if (ogg_stream_pagein(&decode_data.o_state, &page) < 0) {
			Error::raise(Error::Status::OggVorbisError, "Failed to read Ogg packet.");
		}
		decode_ogg_packet_out(decode_data, packet);
	}
}

Buffer * OggVorbisDecoder::decode(std::ifstream &in) const {
	RAII_OggDecodeData ogg_decode_data(in);
	RAII_VorbisDecodeData vorbis_decode_data(ogg_decode_data);

	int offset = 0;

	while (! in.eof()) {
		int status;
		ogg_packet packet;

		decode_ogg_packet_out(ogg_decode_data, packet);

		if ((status = vorbis_synthesis(&vorbis_decode_data.v_block, &packet)) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}

		if ((status = vorbis_synthesis_blockin(&vorbis_decode_data.v_dsp, &vorbis_decode_data.v_block)) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}

		float **pcm;
		int count;
		
		while ((count = vorbis_synthesis_pcmout(&vorbis_decode_data.v_dsp, &pcm)) > 0) {
			vorbis_decode_data.buffer->write((unsigned int)offset, (unsigned int)count, (const float **)pcm);
			vorbis_synthesis_read(&vorbis_decode_data.v_dsp, count);
			offset += count;
		}
	}

	Buffer *buffer = vorbis_decode_data.buffer;
	vorbis_decode_data.buffer = nullptr;

	return buffer;
}

//////////////////////////////////////////////////////////////////////////////
// Coder /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct RAII_OggVorbisCoderData {
	std::ofstream &output;
	std::ofstream::iostate output_state;

	ogg_stream_state o_state;

	vorbis_info v_info;
	vorbis_dsp_state v_dsp;
	vorbis_comment v_comment;
	vorbis_block v_block;

	RAII_OggVorbisCoderData(const OggVorbisCoder &ov_coder, const Buffer &buffer, std::ofstream &out) :
		output(out) {

		output_state = output.exceptions();
		output.exceptions(std::ofstream::failbit | std::ofstream::badbit );

		if (ogg_stream_init(&o_state, (int)time(nullptr)) < 0) {
			Error::raise(Error::Status::OggVorbisError, "Ogg internal error.");
		}

		Format format = buffer.format();
		int status;

		vorbis_info_init(&v_info);

		if ((status = vorbis_encode_init_vbr(&v_info, format.channelCount(), format.sampleRate(), vorbis_quality(ov_coder.quality()))) < 0) {
			Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
		}
		
		if (vorbis_analysis_init(&v_dsp, &v_info) != 0) {
			Error::raise(Error::Status::OggVorbisError, "Vorbis internal error.");
		}
		
		if (vorbis_block_init(&v_dsp, &v_block) != 0) {
			Error::raise(Error::Status::OggVorbisError, "Vorbis internal error.");
		}

		vorbis_comment_init(&v_comment);
		vorbis_comment_add_tag(&v_comment, "ENCODER", "nraudio");
		vorbis_comment_add_tag(&v_comment, "VENDOR", "nealrame.com");
	}

	virtual ~RAII_OggVorbisCoderData( ) {
		vorbis_comment_clear(&v_comment);
		vorbis_dsp_clear(&v_dsp);
		vorbis_info_clear(&v_info);
		ogg_stream_clear(&o_state);
		output.exceptions(output_state);
	}
};

void encode_write_ogg_page(ogg_page &page, std::ostream &out) {
	try {
		out.write((const char *)page.header, page.header_len);
		out.write((const char *)page.body,   page.body_len);
	} catch (std::ofstream::failure ioerr) {
		Error::raise(Error::Status::IOError, ioerr.what());
	}
}

void encode_header(RAII_OggVorbisCoderData &encode_data) {
	ogg_packet packet, packet_comm, packet_code;
	ogg_page page;
	int status;

	if ((status = vorbis_analysis_headerout(&encode_data.v_dsp, &encode_data.v_comment, &packet, &packet_comm, &packet_code)) < 0) {
		Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
	}

	if ((ogg_stream_packetin(&encode_data.o_state, &packet) < 0)
		|| (ogg_stream_packetin(&encode_data.o_state, &packet_comm) < 0)
		|| (ogg_stream_packetin(&encode_data.o_state, &packet_code) < 0)) {
		Error::raise(Error::Status::OggVorbisError, "Ogg internal error.");
	}

	while (ogg_stream_flush(&encode_data.o_state, &page)) {
		encode_write_ogg_page(page, encode_data.output);
	}
}

void encode_flush(RAII_OggVorbisCoderData &encode_data) {
	ogg_page page;

	while (ogg_stream_flush(&encode_data.o_state, &page)) {
		encode_write_ogg_page(page, encode_data.output);
	}
}

void encode_samples(RAII_OggVorbisCoderData &encode_data) {
	ogg_packet packet;
	ogg_page page;
	int status;

	if ((status = vorbis_analysis(&encode_data.v_block, &packet)) < 0) {
		Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
	}

	if (ogg_stream_packetin(&encode_data.o_state, &packet) < 0) {
		Error::raise(Error::Status::OggVorbisError, "Ogg internal error.");
	}

	while (ogg_stream_pageout(&encode_data.o_state, &page)) {
		encode_write_ogg_page(page, encode_data.output);
	}
}

void encode_samples(RAII_OggVorbisCoderData &encode_data, unsigned int count) {
	int status;

	if ((status = vorbis_analysis_wrote(&encode_data.v_dsp, count)) < 0) {
		Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
	}

	while ((status = vorbis_analysis_blockout(&encode_data.v_dsp, &encode_data.v_block)) > 0) {
		encode_samples(encode_data);
	}

	if (status < 0) {
		Error::raise(Error::Status::OggVorbisError, vorbis_error_string(status));
	}
}

void OggVorbisCoder::encode(const Buffer &buffer, std::ofstream &out) const {
	RAII_OggVorbisCoderData encode_data(*this, buffer, out);
	unsigned int offset = 0;

	encode_header(encode_data);
	do {
		float **samples = vorbis_analysis_buffer(&encode_data.v_dsp, 1024);
		unsigned int count = buffer.read(offset, 1024, samples);

		encode_samples(encode_data, count);

		offset += count;
	} while (offset < buffer.frameCount());

	encode_samples(encode_data, 0);
	encode_flush(encode_data);
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
