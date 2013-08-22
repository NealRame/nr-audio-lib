CFLAGS   += -O3
CXXFLAGS += -O3

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) -q $@ $(OBJECTS)

include $(DEPS)
