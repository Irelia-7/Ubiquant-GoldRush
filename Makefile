module = player
CC = g++
CFLAGS = -fvisibility-inlines-hidden -fmessage-length=0 -march=native -mtune=haswell -ftree-vectorize -fPIC -fstack-protector-strong -fno-plt -O3 -ffunction-sections -pipe
CFLAGS += -DNDEBUG -D_FORTIFY_SOURCE=2 -O3 -fPIC
LDFLAGS = -lpthread
CXXFLAGS = -std=c++11 -O3
SOURCES = ${module}.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = ${module}.so
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) -shared -o $@ $^ -fPIC -O3
%.o: %.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)