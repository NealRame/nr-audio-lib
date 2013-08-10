CFLAGS   += -O0 -g
CXXFLAGS += -O0 -g

$(TARGET): $(OBJECTS)
	@echo $(OBJECTS)
	$(AR) -q $@ $(OBJECTS)

include $(DEPS)
