CC=g++
NAME=tap

CPPFLAGS = -Iinc
CFLAGS   = -Wall -Wextra -Werror -g3

LDFLAGS  = -Linc/lib
LDLIBS   = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

LIB = inc/lib/

INC = inc/button.hpp \
	 inc/color.hpp \
	 inc/game_world.hpp \
	 inc/game_state.hpp \
	 inc/game_scene.hpp \
	 inc/helper.hpp \
	 inc/new_game.hpp \
	 inc/save_load.hpp \
	 inc/splash_screen.hpp \
	 inc/tap.hpp \
	 inc/server.hpp \

SRC = src/main.cpp \
	 src/helper.cpp \
	 src/tap.cpp \
	 src/save_load.cpp \
	 src/button.cpp \
	 src/game_state.cpp \
	 src/server.cpp \
	 src/scene/game_world/game_world.cpp \
	 src/scene/game_world/connection_error.cpp \
	 src/scene/game_world/connection_loading.cpp \
	 src/scene/game_world/island_1.cpp \
	 src/scene/splash_screen/load_game.cpp \
	 src/scene/splash_screen/new_game.cpp \
	 src/scene/splash_screen/splash_screen.cpp \
	 src/scene/splash_screen/error_screen.cpp \
	 src/scene/game_world/net_update.cpp \

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

BIN_DIR = bin

BLUE = \033[1;34m
GREEN = \033[1;32m
RESET = \033[0m
YELLOW = \033[1;33m

all: build-raylib $(NAME)

run: build-raylib $(NAME)
	./$(NAME)

build-raylib:
	cd raylib/src && make
	cp raylib/src/libraylib.a inc/lib/.

$(NAME): $(OBJ)
	@$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)
	@printf "$(GREEN)Tap gui Build successful!$(RESET)\n"

$(OBJ): $(OBJ_DIR)/%.o: %.cpp $(INC) Makefile
	@mkdir -p $(dir $@)
	@printf "$(BLUE)[+] Compiling $(RESET)$<\n"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@cd raylib/src && make clean
	@printf "$(YELLOW) raylib object\n"
	rm -fr $(OBJ) $(OBJ_DIR)
	@printf "$(YELLOW)[-] Delete $(OBJ) $(RESET)\n"

debug:
	gdb $(NAME)

fclean: clean
	@rm -fr $(NAME)
	@printf "$(YELLOW)[-] Delete $(NAME) binary$(RESET)\n"

re: fclean build-raylib $(NAME)

lint:
	cpplint --filter=-legal/copyright --recursive .

.PHONY: all clean fclean re
