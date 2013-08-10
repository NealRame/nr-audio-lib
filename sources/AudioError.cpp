/*
 * Error.cpp
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#include "AudioError.h"

namespace com {
namespace nealrame {
namespace audio {

Error::Error(Error::Status s, std::string msg) : status(s), message(msg) {
}

Error::~Error() {
}

void Error::raise(Status status, std::string message) {
	throw Error(status, message);
}

std::string Error::statusToString(Error::Status status) {
	switch (status) {
	case Status::Success:
		return "audio::Success";

	case Status::IOError:
		return "audio::IOError";

	case Status::LameDecodingFailed:
		return "audio::LameDecodingFailed";

	case Status::LameInitializationFailed:
		return "audio::LameInitializationFailed";

	case Status::NoSuitableDecoder:
		return "audio::NoSuitableDecoder";

	case Status::NotImplemented:
		return "audio::NotImplemented";

	case Status::UndefinedFormat:
		return "audio::UndefinedFormat";
	}
	return "audio:UnknownError";
}

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
