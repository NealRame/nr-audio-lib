/**
 * # AudioCoder.h
 *
 * Created on: Jun 18, 2013
 * Author: [NealRame](mailto:contact@nealrame.com)
 */

#ifndef AUDIOCODER_H_
#define AUDIOCODER_H_

#include <fstream>
#include <string>

namespace com {
namespace nealrame {
namespace audio {
class Buffer;
/**
 * ## Class coder
 */
class Coder {
public:
	/**
	 * ### Encoding quality
	 * The enum class `Quality` defines four choice of quality sorted from
	 * best quality to worst quality or in other words from slowest to
	 * fastest algorithm :
	 * * `Coder::Quality::Best`:       best quality, encoding speed does not matter,
	 * * `Coder::Quality::Good`:       good quality but not too slow,
	 * * `Coder::Quality::Acceptable`: fast encoding speed with acceptable quality,
	 * * `Coder::Quality::Fastest`:    encode as fast as possible, quality does not matter.
	 */
	enum class Quality {
		Best,
		Good,
		Acceptable,
		Fastest,
	};

public:
	/** ------------------------------------------------------------------
	 * ### Constructors
	 */

	/**
	 * * `Coder()` 
	 *     Build a Coder with default quality (default is `Quality::Good`, see section **Encoding quality**).
	 */
	Coder();
	/**
	 * * `Coder(Quality)`
	 *     Build a Coder with the given quality.
	 */
	Coder(Quality);

	virtual ~Coder() {}

public:
	/**-------------------------------------------------------------------
	 * ### Methods
	 */

	/**
	 * * `Quality quality() const`
	 *     Get this `Coder` quality.
	 */
	Quality quality() const;
	/**
	 * * `void setQuality(Quality)`
	 *     Set this `Coder` quality to the given one.
	 */
	void setQuality(Quality);

	/**
	 * * `void encode(const Buffer &, const std::string &) const`
	 *     Encode the given buffer to the given filename (see [Buffer.md](doc/Buffer.md)for more details about `Buffer`).
	 */
	virtual void encode(const Buffer &, const std::string &) const;

	/**
	 * * `void encode(const Buffer &, const std::string &) const`
	 *     Encode the given buffer to the given output stream (see [Buffer.md](doc/Buffer.md)for more details about `Buffer`).
	 */
	virtual void encode(const Buffer &, std::ofstream &) const = 0;

private:
	Quality _quality;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* AUDIOCODER_H_ */
