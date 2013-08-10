export TARGET        = libnraudio.a
export CC            = g++
export AR	     = ar
export COMMON_FLAGS  = -Wall -Werror
export CFLAGS        = $(COMMON_FLAGS)
export CXXFLAGS      = -std=c++11 $(COMMON_FLAGS)
export SOURCES      := $(wildcard $(PWD)/sources/*.cpp)
export SOURCES      += $(wildcard $(PWD)/sources/codec/*.cpp)
export OBJECTS      := $(notdir $(patsubst %.cpp,%.o,$(SOURCES)))
export VPATH        := $(PWD)/sources:$(PWD)/sources/codec
export DEPS         := $(PWD)/Makefile.depends

.PHONY: all clean Debug depends realclean Release tags

all: Debug Release

Debug:
	@mkdir -p $@
	$(MAKE) --no-print-directory -C $@ -f ../$@.mk $(TARGET)

Release:
	@mkdir -p $@
	$(MAKE) --no-print-directory -C $@ -f ../$@.mk $(TARGET)

tags:
	$(CC) -M $(INCLUDE_DIRECTORIES) $(SOURCES) \
		| sed -e 's/[\\ ]/\n/g' \
		| sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d' \
		| ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q

depends: $(SOURCES)
	$(CC) $(CXXFLAGS) $(INCLUDE_DIRECTORIES) -MM $(SOURCES) > $(DEPS)

realclean: clean
	rm -fr log
	rm -fr tags
	rm -fr Debug
	rm -fr Release

clean:
	rm -fr *~
	rm -fr $(DEPS)
