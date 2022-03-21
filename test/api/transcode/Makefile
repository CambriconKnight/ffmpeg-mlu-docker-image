
CFLAGS=-I /usr/local/neuware/include/ -I /usr/local/include -g -std=c++11
CXXFLAGS=$(CFLAGS)

LDFALGS=-L /usr/local/neuware/lib64 -L /usr/local/lib

CXX=g++

SRCS=$(wildcard ./src/*.cpp)
OBJS=$(SRCS:%.cpp=%.o)

TARGET=transcode

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFALGS) -lpthread -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil

clean:
	rm  -f $(TARGET) $(OBJS)


