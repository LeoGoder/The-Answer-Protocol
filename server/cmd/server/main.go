package main

import (
	"bufio"
	"fmt"
	"net"

	"tap/server/internal/game"
	"tap/server/internal/protocol"
)

func send(conn net.Conn, message string) bool {
	if _, err := conn.Write([]byte(message + "\n")); err != nil {
		fmt.Println("Write error:", err)
		return false
	}
	return true
}

func handleClient(conn net.Conn, state *game.State) {
	username := ""

	defer func() {
		if username != "" {
			state.RemovePlayer(username)
			fmt.Println("Disconnected user:", username)
		}
		conn.Close()
	}()

	fmt.Println("Client connected:", conn.RemoteAddr())
	if !send(conn, protocol.OKHello) {
		return
	}

	reader := bufio.NewReader(conn)
	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Client disconnected:", conn.RemoteAddr())
			return
		}

		cmd := protocol.Parse(line)
		if cmd.Name == "" {
			continue
		}
		if err := protocol.Validate(cmd); err != nil {
			fmt.Println("Invalid syntax:", err)
			if !send(conn, protocol.ErrBadRequest) {
				return
			}
			continue
		}

		fmt.Println("Command:", cmd.Name)
		fmt.Println("Args:", cmd.Args)

		switch cmd.Name {
		case "QUIT":
			send(conn, protocol.OKBye)
			return

		case "CONNECT":
			if username != "" {
				if !send(conn, protocol.ErrBadRequest) {
					return
				}
				continue
			}

			requested := cmd.Args[0]
			if !state.AddPlayer(requested) {
				if !send(conn, protocol.ErrNameInUse) {
					return
				}
				continue
			}

			username = requested
			fmt.Println("Authenticated:", username)
			if !send(conn, protocol.OKConnected) {
				return
			}

		case "LOOK", "MOVE", "CHAT", "WHO", "GROUP",
			"TAKE", "DROP", "INVENTORY", "TALK",
			"ATTACK", "STATUS", "QUEST", "QUESTS":

			if username == "" {
				fmt.Println("Game command rejected: client not authenticated")
				if !send(conn, protocol.ErrNotAuthenticated) {
					return
				}
				continue
			}

			response := state.Handle(username, cmd)
			if response == "" {
				response = protocol.ErrBadRequest
			}
			if !send(conn, response) {
				return
			}

		default:
			fmt.Println("Unknown command:", cmd.Name)
			if !send(conn, protocol.ErrUnknownCommand) {
				return
			}
		}
	}
}

func main() {
	fmt.Println("TAP server starting...")

	state, err := game.LoadState()
	if err != nil {
		fmt.Println("World load error:", err)
		return
	}
	fmt.Println("World loaded successfully")

	listener, err := net.Listen("tcp", ":4242")
	if err != nil {
		fmt.Println("Listen error:", err)
		return
	}
	defer listener.Close()

	fmt.Println("Server listening on port 4242")
	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}
		go handleClient(conn, state)
	}
}
