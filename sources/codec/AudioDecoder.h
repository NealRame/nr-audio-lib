/*
 * AudioCodec.h
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

#ifndef AUDIODECODER_H_
#define AUDIODECODER_H_

#include <fstream>
#include <memory>
#include <string>

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class Decoder {
public:
	static std::shared_ptr<Decoder> getDecoder(const std::string file_extension);
public:
	virtual ~Decoder() {}
public:
	virtual Buffer * decode(const std::string &) const;
	virtual Buffer * decode(std::ifstream &) const = 0;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOCODEC_H_ */
