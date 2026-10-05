CXX = g++
TARGET = tap_client_tui

# --- FTXUI (téléchargé et compilé automatiquement dans lib/) ---
FTXUI_DIR = lib/FTXUI
FTXUI_BUILD = $(FTXUI_DIR)/build
FTXUI_LIB = $(FTXUI_BUILD)/libftxui-component.a

# -isystem : évite d'afficher les warnings venant des headers de FTXUI
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -MMD -MP \
           -I./includes -Isrc -I./lib -isystem $(FTXUI_DIR)/include
LDFLAGS = -L$(FTXUI_BUILD)
LDLIBS = -lftxui-component -lftxui-dom -lftxui-screen

SRC = src/main.cpp src/connexion.cpp src/image.cpp src/widget.cpp src/parssing.cpp
OBJ = $(SRC:src/%.cpp=.obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS) $(LDLIBS)

.obj/%.o: src/%.cpp | $(FTXUI_LIB)
	@mkdir -p .obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clone + compile FTXUI (une seule fois)
$(FTXUI_LIB):
	@test -d $(FTXUI_DIR) || git clone --depth 1 https://github.com/ArthurSonzogni/FTXUI.git $(FTXUI_DIR)
	cmake -S $(FTXUI_DIR) -B $(FTXUI_BUILD) -DCMAKE_BUILD_TYPE=Release \
		-DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF -DFTXUI_BUILD_TESTS=OFF
	cmake --build $(FTXUI_BUILD) -j4

run-client: all
	./$(TARGET)

clean:
	rm -rf .obj $(TARGET)

fclean: clean
	rm -rf $(FTXUI_DIR)

run: all run-client

re: clean run-client

lint:
	cpplint --filter=-legal/copyright --recursive src includes


-include $(OBJ:.o=.d)


.PHONY: all clean fclean run lint run-client re