/*
 * AudioCoder.cpp
 *
 *  Created on: Jun 18, 2013
 *      Author: jux
 */

#include "../AudioError.h"
#include "AudioCoder.h"

namespace com {
namespace nealrame {
namespace audio {

void Coder::encode(const Buffer &buffer, const std::string &filename) const {
	std::ofstream ofs(filename.data(), std::ofstream::binary);
	try {
		encode(buffer, ofs);
		ofs.close();
	} catch (Error &e) {
		throw e;
		ofs.close();
	}
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
