/*
 * Buffer.h
 *
 *  Created on: May 20, 2013
 *      Author: jux
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include "AudioFormat.h"

namespace com {
namespace nealrame {
namespace audio {

class Buffer {
public:
	Buffer(Format);
	Buffer(Format, size_t size, char *samples);
	virtual ~Buffer();

public:
	bool isNull() const;
	Format format() const;

public:
	unsigned int frameCount() const;
	double duration() const;

public:
	const char * data() const;

	unsigned int read(unsigned int offset, unsigned int count, float *dst) const;
	unsigned int read(unsigned int offset, unsigned int count, int8_t *dst) const;
	unsigned int read(unsigned int offset, unsigned int count, int16_t *dst) const;

	void write(unsigned int offset, unsigned int count, const int8_t *);
	void write(unsigned int offset, unsigned int count, const int16_t *);
	void write(unsigned int offset, unsigned int count, const float  *);

	unsigned int read(unsigned int offset, unsigned int count, int8_t **dst) const;
	unsigned int read(unsigned int offset, unsigned int count, int16_t **dst) const;
	unsigned int read(unsigned int offset, unsigned int count, float **) const;

	void write(unsigned int offset, unsigned int count, const int8_t **);
	void write(unsigned int offset, unsigned int count, const int16_t **);
	void write(unsigned int offset, unsigned int count, const float **);

	void resize(unsigned int frameCount);

private:
	size_t readFrame (const char *src, float  *dst) const;
	size_t readFrame (const char *src, float **dst, unsigned int frame_index) const;

private:
	Format _format;
	unsigned int _frameCount;
	char *_samples;
};

} /* namespace audio */
} /* namespace nealrame */
} /* namespace com */
#endif /* BUFFER_H_ */
