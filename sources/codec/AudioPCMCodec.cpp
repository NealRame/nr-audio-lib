/*
 * PCMCodec.cpp
 *
 *  Created on: May 28, 2013
 *      Author: jux
 */

#include <cstdint>
#include <cstring>
#include <iostream>

#include "../AudioBuffer.h"
#include "../AudioError.h"

#include "AudioPCMCoder.h"
#include "AudioPCMDecoder.h"

namespace com {
namespace nealrame {
namespace audio {

struct RIFFHeaderChunk {
	char id[4];
	uint32_t size;
	char format[4];
} __attribute__((packed));

struct WaveFormatChunk {
	char id[4];
	uint32_t size;
	uint16_t audioFormat;
	uint16_t channelCount;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t bytePerFrame;
	uint16_t bitPerSample;
} __attribute__((packed));

struct WaveDataChunk {
	char id[4];
	uint32_t size;
} __attribute__((packed));

#if defined (DEBUG)
#define debug_riff_header_chunk(CHUNK) \
	do { \
		std::cerr << "RIFF header chunk:" << std::endl \
			<< "  size: "             << (CHUNK).size << std::endl; \
	} while (0)
#define debug_wave_format_chunk(CHUNK) \
	do { \
		std::cerr << "WAVE format chunk:" << std::endl \
			<< "  size: "             << (CHUNK).size << std::endl \
			<< "  audio format: "     << (CHUNK).audioFormat << std::endl \
			<< "  channel count: "    << (CHUNK).channelCount << std::endl \
			<< "  sample rate: "      << (CHUNK).sampleRate << std::endl \
			<< "  byte rate: "        << (CHUNK).byteRate << std::endl \
			<< "  byte per frame: "   << (CHUNK).bytePerFrame << std::endl \
			<< "  bit per sample: "   << (CHUNK).bitPerSample << std::endl; \
	} while (0)
#define debug_wave_data_chunk(CHUNK) \
	do { \
		std::cerr << "WAVE data chunk:" << std::endl \
			<< "  size: "           << (CHUNK).size << std::endl; \
	} while (0)
#else
#define debug_riff_header_chunk(...)
#define debug_wave_format_chunk(...)
#define debug_wave_data_chunk(...)
#endif

//////////////////////////////////////////////////////////////////////////////
// Decoder ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct RAII_PCMDecoderData {
	std::ifstream &input;
	std::ifstream::iostate input_state;
	char *samples;

	RAII_PCMDecoderData(std::ifstream &in) :
		input(in) {
		samples = nullptr;
		input_state = input.exceptions();
		input.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	}

	virtual ~RAII_PCMDecoderData() {
		input.exceptions(input_state);
		if (samples != nullptr) free(samples);
	}
};

Buffer * PCMDecoder::decode(std::ifstream &in) const {
	RAII_PCMDecoderData decode_data(in);
	Buffer *buffer = nullptr;

	try {
		RIFFHeaderChunk header_chunk;
		in.readsome((char *)&header_chunk, sizeof(RIFFHeaderChunk));
		if (strncmp(header_chunk.id, "RIFF", 4) != 0
			&& strncpy(header_chunk.format, "WAVE", 4) != 0) {
			Error::raise(Error::Status::PCMError, "Bad file format.");
		}
		debug_riff_header_chunk(header_chunk);

		WaveFormatChunk format_chunk;
		in.readsome((char *)&format_chunk, sizeof(WaveFormatChunk));
		if (strncmp(format_chunk.id, "fmt ", 4) != 0) {
			Error::raise(Error::Status::PCMError, "Bad file format.");
		}
		debug_wave_format_chunk(format_chunk);

		WaveDataChunk data_chunk;
		in.readsome((char *)&data_chunk, sizeof(WaveDataChunk));
		if (strncmp(data_chunk.id, "data", 4) != 0) {
			Error::raise(Error::Status::PCMError, "Bad file format.");
		}
		debug_wave_data_chunk(data_chunk);

		Format format(format_chunk.channelCount, format_chunk.sampleRate, format_chunk.bitPerSample);

		decode_data.samples = (char *) malloc(data_chunk.size);

		in.read(decode_data.samples, data_chunk.size);
		buffer = new Buffer(format, data_chunk.size, decode_data.samples);
		decode_data.samples = nullptr;

	} catch (std::ifstream::failure ioerr) {
		Error::raise(Error::Status::IOError, ioerr.what());
	}

	return buffer;
}

//////////////////////////////////////////////////////////////////////////////
// Coder /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct RAII_PCMCoderData {
	std::ofstream &output;
	std::ofstream::iostate output_state;

	RAII_PCMCoderData(std::ofstream &out) :
		output(out) {
		output_state = output.exceptions();
		output.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	}

	virtual ~RAII_PCMCoderData() {
		output.exceptions(output_state);
	}
};

void PCMCoder::encode(const Buffer &buffer, std::ofstream &out) const {
	RIFFHeaderChunk header_chunk;

	try {
		Format fmt = buffer.format();

		memcpy(header_chunk.id,     "RIFF", 4);
		memcpy(header_chunk.format, "WAVE", 4);
		header_chunk.size = 4 + sizeof(WaveFormatChunk) + sizeof(WaveDataChunk) + fmt.sizeForFrameCount(buffer.frameCount());
		debug_riff_header_chunk(header_chunk);
		out.write((const char *)&header_chunk, sizeof(RIFFHeaderChunk));

		WaveFormatChunk wave_format;
		memcpy(wave_format.id, "fmt ", 4);
		wave_format.size = sizeof(WaveFormatChunk) - sizeof(wave_format.id) - sizeof(wave_format.size);
		wave_format.audioFormat = 1;
		wave_format.channelCount = fmt.channelCount();
		wave_format.sampleRate = fmt.sampleRate();
		wave_format.byteRate = fmt.sizeForFrameCount(fmt.sampleRate());
		wave_format.bytePerFrame = fmt.sizeForFrameCount(1);
		wave_format.bitPerSample = fmt.bitDepth();
		debug_wave_format_chunk(wave_format);
		out.write((const char *)&wave_format, sizeof(WaveFormatChunk));

		WaveDataChunk wave_data;
		memcpy(wave_data.id, "data", 4);
		wave_data.size = fmt.sizeForFrameCount(buffer.frameCount());
		debug_wave_data_chunk(wave_data);
		out.write((const char *)&wave_data, sizeof(WaveDataChunk));
		out.write((const char *)buffer.data(), fmt.sizeForFrameCount(buffer.frameCount()));
	} catch (std::ofstream::failure ioerr) {
		Error::raise(Error::Status::IOError, ioerr.what());
	}
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
