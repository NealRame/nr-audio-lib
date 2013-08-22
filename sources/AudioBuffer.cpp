/*
 * Buffer.cpp
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */
extern "C" {
#	include <memory.h>
}
#include <fstream>
#include <string>
#include <limits>

#include "AudioBuffer.h"
#include "AudioError.h"

#include <iostream>

namespace com {
namespace nealrame {
namespace audio {

template<typename SOURCE, typename DEST>
struct Resampler {
	static inline DEST value(SOURCE v) {
		return Resampler<SOURCE, float>::value(v)*std::numeric_limits<DEST>::max();
	}
};

template<typename SOURCE>
struct Resampler<SOURCE, SOURCE> {
	static inline SOURCE value(SOURCE v) {
		return v;
	}
};

template<typename SOURCE>
struct Resampler<SOURCE, float> {
	static inline float value(SOURCE v) {
		return static_cast<float>(v)/std::numeric_limits<SOURCE>::max();
	}
};

template<typename DEST>
struct Resampler<float, DEST> {
	static inline DEST value(float v) {
		return v*std::numeric_limits<DEST>::max();
	}
};

Buffer::Buffer(Format format) :
	_format(format),
	_frameCount(0),
	_samples(nullptr) {
}

Buffer::Buffer(Format format, size_t size, char *samples) :
	_format(format) {
	_frameCount = format.frameCountForSize(size);
	_samples = samples;
}

Buffer::~Buffer() {
	if (! isNull()) {
		delete _samples;
	}
}

bool Buffer::isNull() const {
	return _samples == nullptr;
}

Format Buffer::format() const {
	return _format;
}

unsigned int Buffer::frameCount() const {
	return _frameCount;
}

double Buffer::duration() const {
	return _format.durationForFrameCount(_frameCount);
}

size_t Buffer::readFrame(const char *src, float *dst) const {
	size_t size = 0;
	for (unsigned int i = 0, count = _format.channelCount(), depth = _format.bitDepth(); i< count; ++i) {
		switch (depth) {
		case 8:
			dst[i] = static_cast<float>(src[i])/std::numeric_limits<int8_t>::max();
			size += 1;
			break;
		case 16:
			dst[i] = static_cast<float>(*((int16_t *)src + i))/std::numeric_limits<int16_t>::max();
			size += 2;
			break;
		}
	}
	return size;
}

size_t Buffer::readFrame(const char *src, float **dst, unsigned int frame_index) const {
	size_t size = 0;
	for (unsigned int i = 0, count = _format.channelCount(), depth = _format.bitDepth(); i < count; ++i) {
		switch (depth) {
		case 8:
			dst[i][frame_index] = static_cast<float>(src[i])/std::numeric_limits<int8_t>::max();
			break;
		case 16:
			dst[i][frame_index] = static_cast<float>(*((int16_t *)src + i))/std::numeric_limits<int16_t>::max();
			break;
		}
	}
	return size;
}

const char * Buffer::data() const {
	return _samples;
}

template<typename SOURCE, typename DEST>
void _transfer(unsigned int frameCount, unsigned int channelCount, const SOURCE *src, DEST *dst) {
	for (uint i = 0, count = channelCount*frameCount; i < count; i += channelCount) {
		for (uint j = 0; j < channelCount; ++j) {
			dst[i + j] = Resampler<SOURCE, DEST>::value(src[i + j]);
		}
	}
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, int8_t *dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;
	}
	return frameCount;
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, int16_t *dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;
	}
	return frameCount;
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, float *dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int16_t *)ptr, dst);
		break;
	}
	return frameCount;
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const int8_t *src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const int16_t *src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const float *src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

template<typename SOURCE, typename DEST>
void _transfer(unsigned int frameCount, unsigned int channelCount, const SOURCE *src, DEST **dst) {
	for (uint i = 0; i < frameCount; ++i) {
		for (uint j = 0; j < channelCount; ++j) {
			dst[j][i] = Resampler<SOURCE, DEST>::value(src[i*channelCount + j]);
		}
	}
}

template<typename SOURCE, typename DEST>
void _transfer(unsigned int frameCount, unsigned int channelCount, const SOURCE **src, DEST *dst) {
	for (uint i = 0; i < frameCount; ++i) {
		for (uint j = 0; j < channelCount; ++j) {
			dst[i*channelCount + j] = Resampler<SOURCE, DEST>::value(src[j][i]);
		}
	}
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, int8_t **dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples +_format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int16_t *)ptr, dst);
		break;
	}
	return frameCount;
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, int16_t **dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples +_format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int16_t *)ptr, dst);
		break;
	}
	return frameCount;
}

unsigned int Buffer::read(unsigned int offset, unsigned int frameCount, float **dst) const {
	if ((offset + frameCount) > _frameCount) {
		frameCount = (offset < _frameCount) ? _frameCount - offset : 0;
	}
	char *ptr = _samples +_format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, (int8_t *)ptr, dst);
		break;

	case 16:
		_transfer(frameCount, channel_count, (int16_t *)ptr, dst);
		break;
	}
	return frameCount;
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const int8_t **src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const int16_t **src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

void Buffer::write(unsigned int offset, unsigned int frameCount, const float **src) {
	if ((offset + frameCount) > _frameCount) {
		resize(offset + frameCount);
	}
	char *ptr = _samples + _format.sizeForFrameCount(offset);
	unsigned int channel_count = _format.channelCount();
	switch (_format.bitDepth()) {
	case 8:
		_transfer(frameCount, channel_count, src, (int8_t *)ptr);
		break;

	case 16:
		_transfer(frameCount, channel_count, src, (int16_t *)ptr);
		break;
	}
}

void Buffer::resize(unsigned int count) {
	size_t size = _format.sizeForFrameCount(count);
	_frameCount = count;
	_samples = static_cast<char *>((_samples == NULL) ? malloc(size) : realloc(_samples, size));
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
