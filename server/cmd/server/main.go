package main

import (
	"bufio"
	"fmt"
	"net"

	"tap/server/internal/dispatcher"
	"tap/server/internal/game"
	"tap/server/internal/protocol"
	"tap/server/internal/server"
)

func handleClient(connected net.Conn, registry *server.Registry) {
	authenticated := false
	username := ""

	defer func() {
		if authenticated {
			registry.Remove(username)
			fmt.Println("Disconnected user:", username)
		}

		connected.Close()
	}()

	fmt.Println("Client connected:", connected.RemoteAddr())

	if err := server.Send(connected, "OK hello proto=1"); err != nil {
		fmt.Println("Write error:", err)
		return
	}

	reader := bufio.NewReader(connected)

	for {
		message, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Client disconnected:", connected.RemoteAddr())
			return
		}

		cmd := protocol.Parse(message)

		if cmd.Name == "" {
			continue
		}

		// ========================================
		// VALIDATION
		// ========================================

		if err := protocol.Validate(cmd); err != nil {
			fmt.Println("Invalid syntax:", err)

			if err := server.Send(
				connected,
				"ERR 400 BAD_REQUEST",
			); err != nil {
				fmt.Println("Write error:", err)
				return
			}

			continue
		}

		fmt.Println("Command:", cmd.Name)
		fmt.Println("Args:", cmd.Args)

		target := dispatcher.Route(cmd)

		switch target {

		// ========================================
		// SESSION COMMANDS
		// ========================================

		case dispatcher.Session:
			switch cmd.Name {

			case "CONNECT":
				if authenticated {
					if err := server.Send(
						connected,
						"ERR 201 NAME_IN_USE",
					); err != nil {
						fmt.Println("Write error:", err)
						return
					}

					continue
				}

				requestedUsername := cmd.Args[0]

				if !registry.Add(requestedUsername) {
					if err := server.Send(
						connected,
						"ERR 201 NAME_IN_USE",
					); err != nil {
						fmt.Println("Write error:", err)
						return
					}

					continue
				}

				username = requestedUsername
				authenticated = true

				if err := server.Send(
					connected,
					"OK connected",
				); err != nil {
					fmt.Println("Write error:", err)
					return
				}

				fmt.Println("Authenticated:", username)

			case "QUIT":
				if err := server.Send(
					connected,
					"OK bye",
				); err != nil {
					fmt.Println("Write error:", err)
				}

				return
			}

		// ========================================
		// GAME COMMANDS
		// ========================================

		case dispatcher.Game:
			if !authenticated {
				fmt.Println(
					"Game command rejected: client not authenticated",
				)
				continue
			}

			response := game.Handle(
				cmd,
				registry.Count(),
			)

			if response == "" {
				fmt.Println(
					"No response for command:",
					cmd.Name,
				)
				continue
			}

			if err := server.Send(
				connected,
				response,
			); err != nil {
				fmt.Println("Write error:", err)
				return
			}

		// ========================================
		// UNKNOWN COMMAND
		// ========================================

		case dispatcher.Invalid:
			fmt.Println(
				"Invalid command:",
				cmd.Name,
			)
		}
	}
}

func main() {
	fmt.Println("TAP server starting...")

	registry := server.NewRegistry()

	listener, err := net.Listen(
		"tcp",
		":4242",
	)
	if err != nil {
		fmt.Println("Listen error:", err)
		return
	}
	defer listener.Close()

	fmt.Println("Server listening on port 4242")

	for {
		connected, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}

		go handleClient(
			connected,
			registry,
		)
	}
}
