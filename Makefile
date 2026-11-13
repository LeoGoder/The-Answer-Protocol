SERVER_DIR = server
GUI_DIR = new_gui
TUI_DIR = tap/tui

install:
	cd $(SERVER_DIR) && go mod download

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-client-gui:
	cd $(GUI_DIR) && make run

run-client-tui:
	$(MAKE) -C $(TUI_DIR) run-client

clean:
	cd $(SERVER_DIR) && go clean
	$(MAKE) -C $(GUI_DIR) fclean
	$(MAKE) -C $(TUI_DIR) fclean


.PHONY: install run-server run-client-gui clean