/*
 * Format.h
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#ifndef FORMAT_H_
#define FORMAT_H_

#include <cstdlib>
#include <cstdint>

namespace com {
namespace nealrame {
namespace audio {

class Format {
public:
	Format(unsigned int channel_count, unsigned int sample_rate, unsigned int bit_depth);
	
public:
	unsigned int channelCount() const { return _channelCount; }
	Format & setChannelCount(unsigned int);
	unsigned int sampleRate() const { return _sampleRate; }
	Format & setSampleRate(unsigned int);
	unsigned int bitDepth() const { return _bitDepth; }
	Format & setBitDepth(unsigned int);
	double durationForFrameCount(unsigned int) const;
	unsigned int frameCountForDuration(double) const;
	size_t sizeForFrameCount(unsigned int frameCount) const;
	unsigned int frameCountForSize(size_t size) const;

private:
	unsigned int _channelCount;
	unsigned int _sampleRate;
	unsigned int _bitDepth;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* FORMAT_H_ */
