BUILD_DIR := ./build
SRC_DIRS := ./src
EXECUTABLE := ./bin/maxmodels

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIRS)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")
EXTERNAL_SOLVER_PATH := $(abspath vendor/wmaxcdcl/WMaxCDCL2024/code/simp/wmaxcdcl_static)

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++20 -MMD -MP -fopenmp -O3
CXXFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CXXFLAGS += -DEXTERNAL_SOLVER_PATH=\"$(EXTERNAL_SOLVER_PATH)\"
LDFLAGS := -lpthread -ldl -fopenmp

WMAXCDCL_ZIP_URL := https://maxsat-evaluations.github.io/2024/mse24-solver-src/exact/weighted/WMaxCDCL2024.zip
WMAXCDCL_ZIP := ./vendor/WMaxCDCL2024.zip
WMAXCDCL_DIR := ./vendor/wmaxcdcl

.PHONY: build clean all install_dependencies

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
	@echo "[clean] Successfully removed build artifacts."

install_dependencies:
	@echo -n "[dependencies] Removing previous WMaxCDCL2024.zip and $(WMAXCDCL_DIR) if they exist ..."
	@rm -rf $(WMAXCDCL_ZIP) $(WMAXCDCL_DIR) && echo " complete."
	@echo -n "[dependencies] Downloading WMaxCDCL2024.zip ..."
	@mkdir -p ./vendor
	@curl -sSL $(WMAXCDCL_ZIP_URL) -o $(WMAXCDCL_ZIP) && echo " complete."
	@echo -n "[dependencies] Extracting WMaxCDCL2024.zip to $(WMAXCDCL_DIR) ..."
	@mkdir -p $(WMAXCDCL_DIR)
	@unzip -o -q $(WMAXCDCL_ZIP) -d $(WMAXCDCL_DIR) && echo " complete."
	@echo -n "[dependencies] Removing WMaxCDCL2024.zip ..."
	@rm -rf $(WMAXCDCL_ZIP) && echo " complete."
	@echo -n "[dependencies] Compiling WMaxCDCL ..."
	@cd $(WMAXCDCL_DIR)/WMaxCDCL2024/code/simp && make clean >/dev/null && make rs >/dev/null && echo " complete."
	@echo "[dependencies] Successfully installed dependencies. You can now build maxmodels. Use 'make clean build'."
