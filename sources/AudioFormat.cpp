/*
 * Format.cpp
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#include "AudioFormat.h"

namespace com {
namespace nealrame {
namespace audio {

double Format::durationForFrameCount(unsigned int frameCount) const {
	return static_cast<double>(frameCount)/(double)sampleRate;
}

unsigned int Format::frameCountForDuration(double duration) const {
	return static_cast<unsigned int>(sampleRate*duration);
}

size_t Format::sizeForFrameCount(unsigned int frameCount) const {
	return static_cast<size_t>(frameCount*channelCount*(bitDepth/8));
}

unsigned int Format::frameCountForSize(size_t size) const {
	return size/(channelCount*(bitDepth/8));
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
