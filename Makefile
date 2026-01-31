BUILD_DIR := ./build
SRC_DIRS := ./src
EXECUTABLE := ./bin/maxmodels

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIRS)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")
EXTERNAL_SOLVER_PATH := $(shell which wmaxcdcl)

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++20 -MMD -MP -fopenmp -O3
CXXFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CXXFLAGS += -DEXTERNAL_SOLVER_PATH=\"$(EXTERNAL_SOLVER_PATH)\"
LDFLAGS := -lpthread -ldl -fopenmp

.PHONY: build clean all

build: $(OBJS)
	@echo -n "[build] Building maxmodels executable ... "
	@start_time=$$(date +%s.%N); \
	$(CXX) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS) 1>/dev/null && \
	end_time=$$(date +%s.%N); \
	elapsed=$$(echo "$$end_time - $$start_time" | bc); \
	echo "OK ($$elapsed seconds)"

$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.cpp
	@echo -n "[build] Compiling $*.cpp ... "
	@start_time=$$(date +%s.%N); \
	$(CXX) $(CXXFLAGS) -c $< -o $@ 1>/dev/null && \
	end_time=$$(date +%s.%N); \
	elapsed=$$(echo "$$end_time - $$start_time" | bc); \
	echo "OK ($$elapsed seconds)"

-include $(DEPS)

all: build

clean:
	@echo -n "[clean] Removing build directory ... "
	@find $(BUILD_DIR) -type f ! -name '.gitkeep' -delete && echo "OK"
	@echo -n "[clean] Removing executable ... "
	@rm -rf $(EXECUTABLE) && echo "OK"
	@echo "[clean] ... successfully removed build artifacts and dependencies."