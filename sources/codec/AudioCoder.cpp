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

Coder::Coder() : 
	Coder(Quality::Good) {
}

Coder::Coder(Quality quality) :
	_quality(quality) {
}

void Coder::encode(const Buffer &buffer, const std::string &filename) const {
	std::ofstream ofs(filename.data(), std::ofstream::binary);
	try {
		encode(buffer, ofs);
		ofs.close();
	} catch (Error &e) {
		ofs.close();
		throw e;
	}
}

Coder::Quality Coder::quality() const {
	return _quality;
}

void Coder::setQuality(Quality quality) {
	_quality = quality;
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
