/*
 * Format.cpp
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#include "AudioFormat.h"
#include "AudioError.h"

namespace com {
namespace nealrame {
namespace audio {

Format::Format(unsigned int channel_count, unsigned int sample_rate, unsigned int bit_depth) {
	setChannelCount(channel_count);
	setSampleRate(sample_rate);
	setBitDepth(bit_depth);
}

Format & Format::setChannelCount(unsigned int c) {
	if (c < 1) {
		Error::raise(Error::Status::FormatBadValue);
	}
	_channelCount = c;
	return *this;
}

Format & Format::setSampleRate(unsigned int sample_rate) {
	switch (sample_rate) {
	case 8000:
	case 16000:
	case 22050:
	case 44100:
	case 48000:
	case 96000:
		_sampleRate = sample_rate;
		break;
	default:
		Error::raise(Error::Status::FormatBadValue);
		break;
	}
	return *this;
}

Format & Format::setBitDepth(unsigned int bit_depth) {
	switch (bit_depth) {
	case  8: case 16:
		_bitDepth = bit_depth;
		break;
	default:
		Error::raise(Error::Status::FormatBadValue);
		break;
	}
	return *this;
}

double Format::durationForFrameCount(unsigned int frameCount) const {
	return static_cast<double>(frameCount)/static_cast<double>(_sampleRate);
}

unsigned int Format::frameCountForDuration(double duration) const {
	return static_cast<double>(_sampleRate)*duration;
}

size_t Format::sizeForFrameCount(unsigned int frameCount) const {
	return static_cast<size_t>(frameCount*_channelCount*(_bitDepth/8));
}

unsigned int Format::frameCountForSize(size_t size) const {
	return size/(_channelCount*(_bitDepth/8));
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
