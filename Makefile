SERVER_DIR = server
GUI_DIR = new_gui
TUI_DIR = tui
MAKEFLAGS += --no-print-directory

install:
	cd $(SERVER_DIR) && go mod download
	@$(MAKE) -C $(TUI_DIR) install

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-client:
	@$(MAKE) -C $(TUI_DIR) run

run-client-gui:
	@$(MAKE) -C $(GUI_DIR) run

lint:
	cd $(SERVER_DIR) && gofmt -l . && test -z "$$(gofmt -l .)" && go vet ./...
	@$(MAKE) -C $(GUI_DIR) lint
	@$(MAKE) -C $(TUI_DIR) lint

clean:
	cd $(SERVER_DIR) && go clean
	@$(MAKE) -C $(GUI_DIR) fclean
	@$(MAKE) -C $(TUI_DIR) fclean

distclean:
	@$(MAKE) -C $(TUI_DIR) distclean

.PHONY: install run-server run-client run-client-gui lint clean distclean
