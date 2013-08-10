/*
 * AudioPCMDecoder.h
 *
 *  Created on: Jun 18, 2013
 *      Author: jux
 */

#ifndef AUDIOPCMDECODER_H_
#define AUDIOPCMDECODER_H_

#include "AudioDecoder.h"

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class PCMDecoder: public Decoder {
public:
	using Decoder::decode;
	virtual Buffer * decode(std::ifstream &) const;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOPCMDECODER_H_ */
