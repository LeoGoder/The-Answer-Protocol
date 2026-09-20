package main

import (
	"bufio"
	"fmt"
	"net"

	"tap/server/internal/dispatcher"
	"tap/server/internal/protocol"
)

func handleClient(connected net.Conn) {
	defer connected.Close()

	fmt.Println("Client connected:", connected.RemoteAddr())

	_, err := connected.Write([]byte("OK hello proto=1\n"))
	if err != nil {
		fmt.Println("Write error:", err)
		return
	}

	authenticated := false
	username := ""

	reader := bufio.NewReader(connected)

	for {
		message, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Client disconnected:", connected.RemoteAddr())
			return
		}

		fmt.Print("Received: ", message)

		cmd := protocol.Parse(message)

		if cmd.Name == "" {
			continue
		}

		target := dispatcher.Route(cmd)

		fmt.Println("Command:", cmd.Name)
		fmt.Println("Args:", cmd.Args)
		fmt.Println("Route:", target)

		switch target {
		case dispatcher.Session:
			switch cmd.Name {

			case "CONNECT":
				if authenticated {
					fmt.Println("Already authenticated as:", username)
					continue
				}

				if len(cmd.Args) != 1 {
					fmt.Println("Invalid CONNECT syntax")
					continue
				}

				username = cmd.Args[0]
				authenticated = true

				_, err := connected.Write([]byte("OK connected\n"))
				if err != nil {
					fmt.Println("Write error:", err)
					return
				}

				fmt.Println("Authenticated:", username)

			case "QUIT":
				_, err := connected.Write([]byte("OK bye\n"))
				if err != nil {
					fmt.Println("Write error:", err)
				}
				return
			}
		}
	}
}

func main() {
	fmt.Println("TAP server starting...")

	listener, err := net.Listen("tcp", ":4242")
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

		go handleClient(connected)
	}
}
