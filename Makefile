SERVER_DIR = server
GUI_DIR = new_gui

install:
	cd $(SERVER_DIR) && go mod download

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server

run-client-gui:
	cd $(GUI_DIR) && make run

clean:
	cd $(SERVER_DIR) && go clean
	$(MAKE) -C $(GUI_DIR) fclean

.PHONY: install run-server run-client-gui clean