SERVER_DIR = server
GUI_DIR = new_gui
TUI_DIR = tui
MAKEFLAGS += --no-print-directory

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

run-client:
	@$(MAKE) -C $(TUI_DIR) run

run-client-gui:
	@$(MAKE) -C $(GUI_DIR) run

lint:
	cd $(SERVER_DIR) && gofmt -l . && test -z "$$(gofmt -l .)" && go vet ./...
	$(MAKE) -C $(GUI_DIR) lint 
	$(MAKE) -C $(TUI_DIR) lint

clean:
	cd $(SERVER_DIR) && gofmt -l . && test -z "$$(gofmt -l .)" && go vet ./...
	@$(MAKE) -C $(GUI_DIR) lint
	@$(MAKE) -C $(TUI_DIR) lint

distclean:
	@$(MAKE) -C $(TUI_DIR) distclean --no-print-directory

.PHONY: install run-server run-client run-client-gui lint clean distclean
