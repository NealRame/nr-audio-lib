#include <exception>
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>

#include <AudioBuffer.h>
#include <codec/AudioMP3Decoder.h>
#include <codec/AudioPCMCoder.h>

using namespace com::nealrame;

int main(int argc, char **argv) {
	audio::MP3Decoder decoder;
	audio::PCMCoder coder;

	if (argc > 1) {
		std::string input(argv[1]);
		std::string output(boost::filesystem::path(input).stem().string() + ".wav");

		try {
			std::shared_ptr<audio::Buffer> buffer(decoder.decode(input));
			coder.encode(*buffer, output);
		} catch (std::exception e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
	}
	return 0;
}