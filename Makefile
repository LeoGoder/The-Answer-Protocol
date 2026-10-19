SERVER_DIR = server

.PHONY: run-server

run-server:
	cd $(SERVER_DIR) && go run ./cmd/server
