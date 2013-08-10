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

Buffer * PCMDecoder::decode(std::ifstream &in) const {
	if (! in.good()) {
		Error::raise(Error::Status::IOError);
	}

	RIFFHeaderChunk header_chunk;

	if (in.readsome((char *)&header_chunk, sizeof(RIFFHeaderChunk)) != sizeof(RIFFHeaderChunk)) {
		return NULL;
	}

	if (strncmp(header_chunk.id, "RIFF", 4) != 0
		&& strncpy(header_chunk.format, "WAVE", 4) != 0) {
		return NULL;
	}

	debug_riff_header_chunk(header_chunk);

	WaveFormatChunk format_chunk;

	if (in.readsome((char *)&format_chunk, sizeof(WaveFormatChunk)) != sizeof(WaveFormatChunk)) {
		return NULL;
	}

	if (strncmp(format_chunk.id, "fmt ", 4) != 0) {
		return NULL;
	}

	debug_wave_format_chunk(format_chunk);

	WaveDataChunk data_chunk;

	if (in.readsome((char *)&data_chunk, sizeof(WaveDataChunk)) != sizeof(WaveDataChunk)) {
		return NULL;
	}

	if (strncmp(data_chunk.id, "data", 4) != 0) {
		return NULL;
	}

	debug_wave_data_chunk(data_chunk);

	Format format;

	format.channelCount = format_chunk.channelCount;

	switch (format_chunk.sampleRate) {
	case Format::SampleRate_8000:
	case Format::SampleRate_16000:
	case Format::SampleRate_22050:
	case Format::SampleRate_44100:
	case Format::SampleRate_48000:
	case Format::SampleRate_96000:
		format.sampleRate = static_cast<Format::SampleRate>(format_chunk.sampleRate);
		break;
	default:
		return NULL;
	}

	switch (format_chunk.bitPerSample) {
	case Format::BitDepth_8:
	case Format::BitDepth_16:
		format.bitDepth = static_cast<Format::BitDepth>(format_chunk.bitPerSample);
		break;
	default:
		return NULL;
	}

	char *samples = new char[data_chunk.size];

	in.read(samples, data_chunk.size);

	if (in && static_cast<uint32_t>(in.gcount()) == data_chunk.size) {
		return new Buffer(format, data_chunk.size, samples);
	}

	delete samples;
	return NULL;
}

void PCMCoder::encode(const Buffer &buffer, std::ofstream &out) const {
	if (! out.good()) {
		Error::raise(Error::Status::IOError);
	}

	Format fmt = buffer.format();

	RIFFHeaderChunk header_chunk;

	memcpy(header_chunk.id,     "RIFF", 4);
	memcpy(header_chunk.format, "WAVE", 4);
	header_chunk.size = 4 + sizeof(WaveFormatChunk) + sizeof(WaveDataChunk) + fmt.sizeForFrameCount(buffer.frameCount());

	debug_riff_header_chunk(header_chunk);
	out.write((const char *)&header_chunk, sizeof(RIFFHeaderChunk));

	WaveFormatChunk wave_format;

	memcpy(wave_format.id, "fmt ", 4);
	wave_format.size = sizeof(WaveFormatChunk) - sizeof(wave_format.id) - sizeof(wave_format.size);
	wave_format.audioFormat = 1;
	wave_format.channelCount = fmt.channelCount;
	wave_format.sampleRate = fmt.sampleRate;
	wave_format.byteRate = fmt.sizeForFrameCount(fmt.sampleRate);
	wave_format.bytePerFrame = fmt.sizeForFrameCount(1);
	wave_format.bitPerSample = fmt.bitDepth;

	debug_wave_format_chunk(wave_format);
	out.write((const char *)&wave_format, sizeof(WaveFormatChunk));

	WaveDataChunk wave_data;

	memcpy(wave_data.id, "data", 4);
	wave_data.size = fmt.sizeForFrameCount(buffer.frameCount());

	debug_wave_data_chunk(wave_data);
	out.write((const char *)&wave_data, sizeof(WaveDataChunk));
	out.write((const char *)buffer.data(), fmt.sizeForFrameCount(buffer.frameCount()));
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
