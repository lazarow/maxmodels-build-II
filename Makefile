CPLEX_LIB_DIR ?=
CPLEX_INC_DIR ?=
DEBUG_LOGGING ?= 0

IPAMIRLIBDIR := ./vendor/incremental-maxhs/src/build/release/lib

BUILD_DIR := ./build
SRC_DIRS := ./src
EXECUTABLE := ./bin/maxmodels

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIRS)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++20 -MMD -MP -fopenmp -O3
ifeq ($(DEBUG_LOGGING), 1)
	CXXFLAGS += -DDEBUG
endif
CXXFLAGS += -I./vendor/incremental-maxhs/src/ipamir
CXXFLAGS += -I./vendor/argparse/argparse
LDFLAGS := -flto=auto -lipamirmaxhs -L$(IPAMIRLIBDIR) -lz -L$(CPLEX_LIB_DIR) -lcplex -lpthread -ldl -fopenmp

.PHONY: prereqs prepare build clean all

prereqs:
	@echo -n "[prereqs] Checking for CPLEX ... "
	@if [ -d "$(CPLEX_LIB_DIR)" ] && [ -d "$(CPLEX_INC_DIR)" ] && [ -f "$(CPLEX_LIB_DIR)/libcplex.a" ] && [ -d "$(CPLEX_INC_DIR)/ilcplex" ]; then \
		echo "OK"; \
	else \
		echo "ERROR: CPLEX configuration is incorrect."; \
		exit 1; \
	fi
	@echo -n "[prereqs] Checking dependencies (git submodules) ... "
	@if git submodule status | grep -qE '(^-|^U)'; then \
		echo "ERROR: There is at least one git submodule that is not initialized or updated. Run 'make prepare' to fix it."; \
		exit 1; \
	else \
		echo "OK"; \
	fi
	@echo -n "[prereqs] Checking if iMaxHS's static library is correctly built ... "
	@if [ ! -f "$(IPAMIRLIBDIR)/libipamirmaxhs.a" ]; then \
		echo "ERROR: iMaxHS's static library is not correctly built. Run 'make prepare' to fix it."; \
		exit 1; \
	else \
		echo "OK"; \
	fi
	@echo "[prereqs] ... successfully checked prerequisites."

prepare:
	@echo "[prepare] Preparing build environment ... "
	@echo -n "[prepare] Updating git submodules ... "
	@git submodule update --init 1>/dev/null && echo "OK"
	@echo -n "[prepare] Copying iMaxHS's header files from src/incremental-maxhs ... "
	@cp -r src/incremental-maxhs/* vendor/incremental-maxhs/src/ipamir && echo "OK"
	@echo -n "[prepare] Building dependencies ... "
	@cd vendor/incremental-maxhs/src && LINUX_CPLEXLIBDIR=$(CPLEX_LIB_DIR) LINUX_CPLEXINCDIR=$(CPLEX_INC_DIR) $(MAKE) ipamir 1>/dev/null && echo "OK"
	@echo "[prepare] ... successfully prepared build environment."

build: $(OBJS)
	@echo -n "[build] Building maxmodels executable ... "
	@start_time=$$(date +%s.%N); \
	$(CXX) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS) 1>/dev/null && \
	end_time=$$(date +%s.%N); \
	elapsed=$$(echo "$$end_time - $$start_time" | bc); \
	echo "OK ($$elapsed seconds)"

rebuild:
	@echo "[rebuild] Rebuilding all object files..."
	@rm -f $(OBJS)
	@make build

$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.cpp
	@echo -n "[build] Compiling $*.cpp ... "
	@start_time=$$(date +%s.%N); \
	$(CXX) $(CXXFLAGS) -c $< -o $@ 1>/dev/null && \
	end_time=$$(date +%s.%N); \
	elapsed=$$(echo "$$end_time - $$start_time" | bc); \
	echo "OK ($$elapsed seconds)"

-include $(DEPS)

all: prepare prereqs build

clean:
	@echo "[clean] Removing build artifacts and dependencies ... "
	@echo -n "[clean] Removing build directory ... "
	@find $(BUILD_DIR) -type f ! -name '.gitkeep' -delete && echo "OK"
	@echo -n "[clean] Removing executable ... "
	@rm -rf $(EXECUTABLE) && echo "OK"
	@echo -n "[clean] Removing dependencies ... "
	@git submodule deinit -f . 1>/dev/null && echo "OK"
	@echo "[clean] ... successfully removed build artifacts and dependencies."