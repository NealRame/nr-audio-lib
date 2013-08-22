#include <exception>
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>

#include <AudioBuffer.h>
#include <AudioError.h>
#include <codec/AudioOggVorbisDecoder.h>
#include <codec/AudioPCMCoder.h>

using namespace com::nealrame;

int main(int argc, char **argv) {
	audio::PCMCoder coder;
	audio::OggVorbisDecoder decoder;

	if (argc > 1) {
		std::string input(argv[1]);
		std::string output(boost::filesystem::path(input).stem().string() + ".wav");

		try {
			std::shared_ptr<audio::Buffer> buffer(decoder.decode(input));
			coder.encode(*buffer, output);
		} catch (std::exception e) {
			std::cerr << e.what() << std::endl;
			return 1;
		} catch (audio::Error e) {
			std::cerr << e.message << std::endl;
		}
	}
	return 0;
}