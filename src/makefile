CXX := g++
INCLUDE_PATH := -I ../include -I ../third_party
CXXFLAGS := -Wall -std=c++11

#### spdlog ###
INCLUDE_PATH += -I ../third_party/spdlog-1.6.1/include

#### redis ###
INCLUDE_PATH += -I /file_storage/hiredis/hiredis-0.14.1

#### asio ###
INCLUDE_PATH += -I ../third_party/asio/include

SRCS := $(wildcard client/*.cpp) $(wildcard common/*.cpp) $(wildcard common/asio_wrapper/*.cpp) $(wildcard proxy/*.cpp sserver/*.cpp) $(wildcard ../third_party/*.cpp)

OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

TARGET := libfilest.a
	
$(TARGET) : $(OBJS)
	@echo "making libfilest.a...";\
	ar crv $@ $^

%.o : %.cpp
	$(CXX) -o $@ -c $< -MMD -MP $(INCLUDE_PATH) $(CXXFLAGS)

-include $(DEPS)

.PHONY : clean
clean :
	@rm -rf $(OBJS) $(DEPS) $(TARGET);

