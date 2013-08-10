/*
 * AudioMP3Coder.h
 *
 *  Created on: Jun 19, 2013
 *      Author: jux
 */

#ifndef AUDIOMP3CODER_H_
#define AUDIOMP3CODER_H_

#include "AudioCoder.h"

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class MP3Coder : public Coder {
public:
	using Coder::encode;
	virtual void encode(const Buffer &, std::ofstream &) const;
};
} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOMP3CODER_H_ */
