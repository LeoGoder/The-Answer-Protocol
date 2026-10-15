CC=gcc
NAME=tap

C_FLAGS = -Wall -Wextra -Werror -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

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

SRC = main.cpp \
	 helper.cpp \
	 tap.cpp \
	 save_load.cpp \
	 button.cpp \
	 game_state.cpp \
	 scene/game_world/game_world.cpp \
	 scene/splash_screen/load_game.cpp \
	 scene/splash_screen/new_game.cpp \
	 scene/splash_screen/splash_screen.cpp \

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

BLUE = \033[1;34m
GREEN = \033[1;32m
RESET = \033[0m
YELLOW = \033[1;33m

all: build-raylib $(NAME)

run:
	./$(NAME)

build-raylib:
	cd raylib/src && make
	cp raylib/src/libraylib.a inc/lib/.

$(NAME): $(OBJ)
	@$(CC) $(C_FLAGS) $(OBJ) -o $(NAME)
	@printf "$(GREEN)Codexion Build successful!$(RESET)\n"

$(OBJ): $(OBJ_DIR)/%.o: %.cpp $(INC) Makefile
	@mkdir -p $(OBJ_DIR)
	@printf "$(BLUE)[+] Compiling $(RESET)$<\n"
	@$(CC) $(C_FLAGS) -c $< -o $@ -g3

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

re: fclean build-raylib build-tap

.PHONY: all clean fclean re
