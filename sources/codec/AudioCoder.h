/*
 * AudioCoder.h
 *
 *  Created on: Jun 18, 2013
 *      Author: jux
 */

#ifndef AUDIOCODER_H_
#define AUDIOCODER_H_

#include <fstream>
#include <string>

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
class Coder {
public:
	virtual ~Coder() {}
public:
	virtual void encode(const Buffer &, const std::string &) const;
	virtual void encode(const Buffer &, std::ofstream &) const = 0;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOCODER_H_ */
