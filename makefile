# for use write "make smth" in terminal
# for example: "make all", "make run" 

BUILD      := build
TESTING    ?= OFF                        # ON or OFF
BUILD_TYPE ?= Release
CTEST_FLAGS ?= --output-on-failure      # or -V for full output

all:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTING=$(TESTING)
	cmake --build $(BUILD) -j

run:
	./$(BUILD)/TrackAnalysis

allTest:
	$(MAKE) all TESTING=ON

runTest:
	cd $(BUILD) && ctest $(CTEST_FLAGS)

clean:
	rm -rf $(BUILD)

.PHONY: all run test clean