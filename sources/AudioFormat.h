/*
 * Format.h
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#ifndef FORMAT_H_
#define FORMAT_H_

#include <cstdlib>

namespace com {
namespace nealrame {
namespace audio {

class Format {
public:
	enum SampleRate {
		SampleRate_8000  =  8000,
		SampleRate_16000 = 16000,
		SampleRate_22050 = 22050,
		SampleRate_44100 = 44100,
		SampleRate_48000 = 48000,
		SampleRate_96000 = 96000
	};

	enum BitDepth {
		BitDepth_8  = 0x08,
		BitDepth_16 = 0x10
	};

public:
	unsigned int channelCount;
	SampleRate sampleRate;
	BitDepth bitDepth;

public:
	double durationForFrameCount(unsigned int) const;
	unsigned int frameCountForDuration(double) const;
	size_t sizeForFrameCount(unsigned int frameCount) const;
	unsigned int frameCountForSize(size_t size) const;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* FORMAT_H_ */
