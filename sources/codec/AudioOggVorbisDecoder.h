/*
 * AudioOggVorbisCodec.h
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

#ifndef AUDIOOGGVORBISDECODER_H_
#define AUDIOOGGVORBISDECODER_H_

#include "AudioDecoder.h"

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class OggVorbisDecoder : public Decoder {
public:
	using Decoder::decode;
	virtual Buffer * decode(std::ifstream &) const;
};
} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOOGGVORBISDECODER_H_ */
