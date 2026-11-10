SERVER_DIR = server
GUI_DIR = new_gui

.PHONY: run-server

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-client-gui:
	cd $(GUI_DIR) && make run
