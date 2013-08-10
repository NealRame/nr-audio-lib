/*
 * AudioMP3Codec.h
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

#ifndef AUDIOMP3DECODER_H_
#define AUDIOMP3DECODER_H_

#include "AudioDecoder.h"

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class MP3Decoder : public Decoder {
public:
	using Decoder::decode;
	virtual Buffer * decode(std::ifstream &) const;
};
} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOMP3CODEC_H_ */
