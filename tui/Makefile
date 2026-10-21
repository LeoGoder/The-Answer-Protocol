CXX = g++
TARGET = tap_client_tui

# --- Colors & Styling ---
RESET   = \033[0m
BOLD    = \033[1m
GREEN   = \033[32m
YELLOW  = \033[33m
BLUE    = \033[34m
MAGENTA = \033[35m
CYAN    = \033[36m
GRAY    = \033[90m

# --- FTXUI (téléchargé et compilé automatiquement dans lib/) ---
FTXUI_DIR = lib/FTXUI
FTXUI_BUILD = $(FTXUI_DIR)/build
FTXUI_LIB = $(FTXUI_BUILD)/libftxui-component.a

# -isystem : évite d'afficher les warnings venant des headers de FTXUI
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -MMD -MP \
           -I./includes -Isrc -I./lib -isystem $(FTXUI_DIR)/include
LDFLAGS = -L$(FTXUI_BUILD)
LDLIBS = -lftxui-component -lftxui-dom -lftxui-screen

SRC = src/main.cpp \
      src/app_state.cpp \
      src/game_view.cpp \
      src/image_renderer.cpp \
      src/login_view.cpp \
      src/map_resolver.cpp \
      src/network_client.cpp \
      src/positioned_widget.cpp \
      src/protocol.cpp \
      src/ui_helpers.cpp
OBJ = $(SRC:src/%.cpp=.obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@printf "  $(MAGENTA)[LD]$(RESET) Linking   $(BOLD)$(GREEN)%-25s$(RESET)\n" "$@"
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS) $(LDLIBS)
	@printf "  $(BOLD)$(GREEN)✔  Build complete: $(TARGET)$(RESET)\n\n"

.obj/%.o: src/%.cpp | $(FTXUI_LIB)
	@mkdir -p .obj
	@printf "  $(BLUE)[CC]$(RESET) Compiling $(CYAN)%-25s$(RESET) $(GRAY)-> $(RESET)%-20s\n" "$<" "$@"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Clone + compile FTXUI (une seule fois)
$(FTXUI_LIB):
	@printf "  $(YELLOW)[FTXUI]$(RESET) Cloning and building FTXUI...\n"
	@test -d $(FTXUI_DIR) || git clone --depth 1 https://github.com/ArthurSonzogni/FTXUI.git $(FTXUI_DIR)
	@cmake -S $(FTXUI_DIR) -B $(FTXUI_BUILD) -DCMAKE_BUILD_TYPE=Release \
		-DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF -DFTXUI_BUILD_TESTS=OFF > /dev/null
	@cmake --build $(FTXUI_BUILD) -j4 > /dev/null
	@printf "  $(GREEN)✔  FTXUI ready.$(RESET)\n"

run-client: all
	@./$(TARGET)

clean:
	@printf "  $(YELLOW)[RM]$(RESET) Cleaning object files...\n"
	@rm -rf .obj $(TARGET)
	@printf "  $(GREEN)✔  Clean finished.$(RESET)\n"

fclean: clean
	@printf "  $(YELLOW)[RM]$(RESET) Removing FTXUI library...\n"
	@rm -rf $(FTXUI_DIR)
	@printf "  $(GREEN)✔  Full clean finished.$(RESET)\n"

run: all run-client

re: clean all

lint:
	@printf "  $(BLUE)[LINT]$(RESET) Running cpplint...\n"
	@cpplint --filter=-legal/copyright --recursive src includes
	@printf "  $(GREEN)✔  Linting passed with 0 errors.$(RESET)\n"

-include $(OBJ:.o=.d)

.PHONY: all clean fclean run lint run-client re