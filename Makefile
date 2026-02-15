# Compiler Settings
CXX      := g++
CXXFLAGS := -std=c++20 -O3 -Wall -Wextra -fpermissive
INCLUDES := -Isrc -Ilibs/imgui -Ilibs/imgui/backends -Ilibs/glad/include -Ilibs/glm -Ilibs/glfw/include

# Libraries (Linux)
LIBS     := -lglfw3 -lGL -ldl -lpthread -lX11

# Directories
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin

# Source Files
SRCS     := $(wildcard $(SRC_DIR)/*.cpp) \
            $(wildcard $(SRC_DIR)/Utils/*.cpp) \
            libs/glad/src/glad.c \
            libs/imgui/imgui.cpp \
            libs/imgui/imgui_draw.cpp \
            libs/imgui/imgui_tables.cpp \
            libs/imgui/imgui_widgets.cpp \
            libs/imgui/backends/imgui_impl_glfw.cpp \
            libs/imgui/backends/imgui_impl_opengl3.cpp

# Object Files
OBJS     := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp, $(SRCS))) \
            $(patsubst %.c, $(OBJ_DIR)/%.o, $(filter %.c, $(SRCS)))

# Target Executable
TARGET   := $(BIN_DIR)/Lenia

# Rules
all: directories $(TARGET) assets

$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $@ $(LIBS)
	@echo "Build Complete. Run with ./$(TARGET)"

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

assets:
	@mkdir -p $(BIN_DIR)/assets/shaders
	@cp -u assets/shaders/* $(BIN_DIR)/assets/shaders/ 2>/dev/null || true
	@mkdir -p $(BIN_DIR)/Initialisation
	@cp -u Initialisation/*.npy $(BIN_DIR)/Initialisation/ 2>/dev/null || true
	@if [ -d colormap ]; then mkdir -p $(BIN_DIR)/colormap && cp -u colormap/* $(BIN_DIR)/colormap/ 2>/dev/null || true; fi

clean:
	@echo "Cleaning..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

run: all
	@cd $(BIN_DIR) && ./Lenia

.PHONY: all clean run directories assets
