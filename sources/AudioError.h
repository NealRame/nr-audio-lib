/*
 * Error.h
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#ifndef AUDIOERROR_H_
#define AUDIOERROR_H_

#include <string>

namespace com {
namespace nealrame {
namespace audio {

class Error {
public:
	enum class Status {
		Success,
		IOError,
		FormatBadValue,
		MP3CodecError,
		OggVorbisError,
		PCMError,
		NoSuitableDecoder,
		NotImplemented,
		UndefinedFormat,
	};

public:
	static std::string statusToString(Status);

public:
	static void raise(Status, std::string message = "");

public:
	Error(Status, std::string msg = "");
	virtual ~Error();

public:
	Status status;
	std::string message;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* ERROR_H_ */
