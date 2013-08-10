/*
 * PCMCodec.h
 *
 *  Created on: May 28, 2013
 *      Author: jux
 */

#ifndef AUDIOPCMCODER_H_
#define AUDIOPCMCODER_H_

#include "AudioCoder.h"

namespace com {
namespace nealrame {
namespace audio {
class PCMCoder : public Coder {
public:
	using Coder::encode;
	virtual void encode(const Buffer &, std::ofstream &) const;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* PCMCODEC_H_ */
