/*
 * AudioOggVorbisCoder.h
 *
 *  Created on: Jun 19, 2013
 *      Author: jux
 */

#ifndef AUDIOOGGVORBISCODER_H_
#define AUDIOOGGVORBISCODER_H_

#include "AudioCoder.h"

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class OggVorbisCoder : public Coder {
public:
	using Coder::encode;
	virtual void encode(const Buffer &, std::ofstream &) const;
};
} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOOGGVORBISCODER_H_ */
