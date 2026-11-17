CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -DWIN32_LEAN_AND_MEAN
INCLUDES = -Iinclude -Ithird_party
LIBS = -lws2_32 -lmswsock -lpqxx -lpq -lssl -lcrypto -lpthread

# Collect all .cpp files
SRCS = $(wildcard src/*.cpp) \
       $(wildcard src/database/*.cpp) \
       $(wildcard src/models/*.cpp) \
       $(wildcard src/repositories/*.cpp) \
       $(wildcard src/services/*.cpp) \
       $(wildcard src/middleware/*.cpp) \
       $(wildcard src/controllers/*.cpp) \
       $(wildcard src/algorithms/*.cpp) \
       $(wildcard src/websocket/*.cpp) \
       $(wildcard src/utils/*.cpp)

OBJS = $(SRCS:.cpp=.o)
TARGET = transitx.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	del /Q /S *.o 2>nul
	del /Q $(TARGET) 2>nul
