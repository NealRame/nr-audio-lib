/*
 * AudioCodec.cpp
 *
 *  Created on: Jun 16, 2013
 *      Author: jux
 */

#include <iostream>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

#include "../AudioError.h"
#include "AudioDecoder.h"
#include "AudioPCMDecoder.h"
#include "AudioMP3Decoder.h"

namespace com {
namespace nealrame {
namespace audio {

std::shared_ptr<Decoder> Decoder::getDecoder(const std::string filename) {
	std::string ext = boost::to_lower_copy(boost::filesystem::path(filename).extension().string());

	if (ext == ".mp3") {
		return std::shared_ptr<Decoder>(new MP3Decoder);
	}

	if (ext == ".wav") {
		return std::shared_ptr<Decoder>(new PCMDecoder);
	}

	throw Error(Error::Status::NoSuitableDecoder);
}

Buffer * Decoder::decode(const std::string &filename) const {
	std::ifstream ifs(filename.data(), std::ifstream::binary);
	Buffer *buffer = nullptr;
	try {
		buffer = decode(ifs);
		ifs.close();
	} catch (Error &e) {
		ifs.close();
		throw e;
	}
	return buffer;
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
