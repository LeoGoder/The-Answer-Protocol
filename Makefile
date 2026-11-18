SERVER_DIR = server
GUI_DIR = new_gui
TUI_DIR = tui
MAKEFLAGS += --no-print-directory

# run-*-quiet: hide the 24-per-second POSITION / ALIVE traffic on screen.
# The server's full log is still written to server/server.log.
SERVER_QUIET_FILTER = "command":"(POSITION|ALIVE)"|"message":"OK( Alive)?"|EVT ROOM POSITION
GUI_QUIET_FILTER = EVT ROOM POSITION|^(\[SERVER\] )?OK( Alive)?$$

install:
	cd $(SERVER_DIR) && go mod tidy
	@$(MAKE) -C $(TUI_DIR) install

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-server-quiet:
	cd $(SERVER_DIR) && go run ./cmd/server | tee server.log | grep --line-buffered -vE '$(SERVER_QUIET_FILTER)'

run-client:
	@$(MAKE) -C $(TUI_DIR) run

run-client-gui:
	@$(MAKE) -C $(GUI_DIR) run

run-client-gui-quiet:
	@$(MAKE) -C $(GUI_DIR) all
	cd $(GUI_DIR) && stdbuf -oL ./tap | grep --line-buffered -vE '$(GUI_QUIET_FILTER)'

lint:
	cd $(SERVER_DIR) && gofmt -l . && test -z "$$(gofmt -l .)" && go vet ./...
	@$(MAKE) -C $(GUI_DIR) lint
	@$(MAKE) -C $(TUI_DIR) lint

clean:
	cd $(SERVER_DIR) && go clean
	@$(MAKE) -C $(GUI_DIR) fclean
	@$(MAKE) -C $(TUI_DIR) fclean
	rm -f $(SERVER_DIR)/server.log

.PHONY: install run-server run-server-quiet run-client run-client-gui run-client-gui-quiet lint clean
