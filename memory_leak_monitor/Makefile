TARGET := memory_leak_monitor

ver := release

LDFLAGS += -llog4cpp -lrt -lpthread
CXXFLAGS += -std=c++11
ifeq ($(VER),debug)
CXXFLAGS += -g
endif


SRC := $(wildcard ./src/*.cpp)

OBJS := $(SRC:.cpp=.o)


$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o ./bin/$@ $(LDFLAGS)

$(OBJS):%.o:%.cpp
	$(CXX) -c -I./inc $(CXXFLAGS) $^ -o $@ $(LDFLAGS)


.PHONY:clean
clean:
	$(RM) ./bin/$(TARGET) $(OBJS)
