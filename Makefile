export TARGET        = libnraudio.a
export CC            = g++
export AR	     = ar
export COMMON_FLAGS  = -Wall -Werror
export CFLAGS        = $(COMMON_FLAGS)
export CXXFLAGS      = -std=c++11 $(COMMON_FLAGS)
export SOURCES      := $(wildcard $(CURDIR)/sources/*.cpp)
export SOURCES      += $(wildcard $(CURDIR)/sources/codec/*.cpp)
export OBJECTS      := $(notdir $(patsubst %.cpp,%.o,$(SOURCES)))
export VPATH        := $(CURDIR)/sources:$(CURDIR)/sources/codec
export DEPS         := $(CURDIR)/Makefile.depends

.PHONY: all clean Debug depends realclean Release tags

all: Debug Release

Debug Release:
	@mkdir -p $@
	$(MAKE) --no-print-directory -C $@ -f ../$@.mk $(TARGET)

tests: test_mp3encode test_mp3decode test_oggencode test_oggdecode

test_mp3encode: Debug/$(TARGET)
	$(CC) -g -O0 $(CXXFLAGS) -I./sources/ -o tests/mp3encode tests/mp3encode.cpp -L./Debug -lnraudio -lmp3lame -lboost_filesystem -lboost_system

test_mp3decode: Debug/$(TARGET)
	$(CC) -g -O0 $(CXXFLAGS) -I./sources/ -o tests/mp3decode tests/mp3decode.cpp -L./Debug -lnraudio -lmp3lame -lboost_filesystem -lboost_system

test_oggencode: Debug/$(TARGET)
	$(CC) -g -O0 $(CXXFLAGS) -I./sources/ -o tests/oggencode tests/oggencode.cpp -L./Debug -lnraudio -lmp3lame -lvorbisenc -lvorbis -lm -logg -lboost_filesystem -lboost_system

test_oggdecode: Debug/$(TARGET)
	$(CC) -g -O0 $(CXXFLAGS) -I./sources/ -o tests/oggdecode tests/oggdecode.cpp -L./Debug -lnraudio -lmp3lame -lvorbisenc -lvorbis -lm -logg -lboost_filesystem -lboost_system

depends: $(SOURCES)
	$(CC) $(CXXFLAGS) $(INCLUDE_DIRECTORIES) -MM $(SOURCES) > $(DEPS)

realclean: clean
	rm -fr log
	rm -fr tags
	rm -fr Debug
	rm -fr Release
	rm -fr tests/mp3decode
	rm -fr tests/mp3encode
	rm -fr tests/oggdecode
	rm -fr tests/oggencode

clean:
	rm -fr *~
	rm -fr $(DEPS)
