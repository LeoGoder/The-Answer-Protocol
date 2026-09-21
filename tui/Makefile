CXX = g++
TARGET = tap_client_tui

# --- FTXUI (téléchargé et compilé automatiquement dans lib/) ---
FTXUI_DIR = lib/FTXUI
FTXUI_BUILD = $(FTXUI_DIR)/build
FTXUI_LIB = $(FTXUI_BUILD)/libftxui-component.a

# -isystem : évite d'afficher les warnings venant des headers de FTXUI
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -MMD -MP \
           -I./includes -Isrc -isystem $(FTXUI_DIR)/include
LDFLAGS = -L$(FTXUI_BUILD)
LDLIBS = -lftxui-component -lftxui-dom -lftxui-screen

SRC = src/main.cpp src/connexion.cpp
OBJ = $(SRC:src/%.cpp=obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS) $(LDLIBS)

obj/%.o: src/%.cpp | $(FTXUI_LIB)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clone + compile FTXUI (une seule fois)
$(FTXUI_LIB):
	@test -d $(FTXUI_DIR) || git clone --depth 1 https://github.com/ArthurSonzogni/FTXUI.git $(FTXUI_DIR)
	cmake -S $(FTXUI_DIR) -B $(FTXUI_BUILD) -DCMAKE_BUILD_TYPE=Release \
		-DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF -DFTXUI_BUILD_TESTS=OFF
	cmake --build $(FTXUI_BUILD) -j4

run-server:
	go run serve.go

run-client: all
	./$(TARGET)

clean:
	rm -rf obj $(TARGET)

# Supprime aussi FTXUI (il sera re-téléchargé au prochain make)
distclean: clean
	rm -rf lib

run: all run-server run-client
	

lint:
	cpplint --filter=-legal/copyright --recursive src

-include $(OBJ:.o=.d)

.PHONY: all clean distclean run lint run-server run-client