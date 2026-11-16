SERVER_DIR = server
GUI_DIR = new_gui
TUI_DIR = tui

# --- Colors & Styling ---
RESET   = \033[0m
BOLD    = \033[1m
GREEN   = \033[32m
YELLOW  = \033[33m
BLUE    = \033[34m
MAGENTA = \033[35m
CYAN    = \033[36m
GRAY    = \033[90m

install:
	cd $(SERVER_DIR) && go mod download
	@$(MAKE) -C $(TUI_DIR) install --no-print-directory

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-client-gui:
	cd $(GUI_DIR) && make run

run-client-tui:
	@$(MAKE) -C $(TUI_DIR) run --no-print-directory

# C++ lint (GUI, TUI) to be added with the team.
lint:
	cd $(SERVER_DIR) && gofmt -l . && test -z "$$(gofmt -l .)" && go vet ./...
	@$(MAKE) -C $(TUI_DIR) lint --no-print-directory

clean:
	cd $(SERVER_DIR) && go clean
	$(MAKE) -C $(GUI_DIR) fclean
	@$(MAKE) -C $(TUI_DIR) fclean --no-print-directory

distclean:
	@$(MAKE) -C $(TUI_DIR) distclean --no-print-directory

.PHONY: install run-server run-client run-client-gui lint clean distclean
