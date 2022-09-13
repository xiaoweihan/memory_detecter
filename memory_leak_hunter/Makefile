FULL_TARGET := libmemory_leak_hunter.so.1.0.0
SONAME := libmemory_leak_hunter.so
ver := release

LDFLAGS += -lunwind -ldl -lrt
CXXFLAGS += -std=c++11
ifeq ($(VER),debug)
CXXFLAGS += -g
endif


SRC := $(wildcard ./src/*.cpp)

OBJS := $(SRC:.cpp=.o)


$(FULL_TARGET):$(OBJS)
	$(CXX) -fPIC -shared $(CXXFLAGS) $^ -Wl,-soname,$(SONAME) -o ./bin/$@ $(LDFLAGS)

$(OBJS):%.o:%.cpp
	$(CXX) -c -fPIC -I./inc $(CXXFLAGS) $^ -o $@ $(LDFLAGS)


.PHONY:clean
clean:
	$(RM) ./bin/libmemory_leak_hunter.so* $(OBJS)
