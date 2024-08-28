CXX = g++ -std=c++17

BIN = bv2nii
SRC_DIR = ./src
BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))
GLM_DIR = /opt/homebrew/Cellar/glm/1.0.1/include/
NIFTI_DIR = ./nifti_clib
ARGUM_DIR = ./argumentum


SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

GCCFLAGS = -I./src
GCCFLAGS += -I$(GLM_DIR) -I$(NIFTI_DIR) -I$(ARGUM_DIR)

LIBS += -L./nifti_clib -lniftiio -lznz -lz
LIBS += -L./argumentum/lib -largumentum

$(BIN): $(OBJS)	
	$(CXX) $(GCCLAGS) $(LIBS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(GCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJS)