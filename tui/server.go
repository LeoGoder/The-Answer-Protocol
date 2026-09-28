package main

import (
	"bufio"
	"fmt"
	"net"
)

func main() {
	listener, err := net.Listen("tcp", ":4242")
	if err != nil {
		fmt.Println("Erreur lors du démarrage du serveur:", err)
		return
	}
	defer listener.Close()
	fmt.Println("Serveur en écoute sur le port 4242...")

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Erreur de connexion:", err)
			continue
		}
		
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()
	fmt.Println("Nouveau client connecté:", conn.RemoteAddr())

	scanner := bufio.NewScanner(conn)
	for scanner.Scan() {
		texteRecu := scanner.Text()
		fmt.Println("Reçu:", texteRecu)

		conn.Write([]byte("OK\n"))
	}

	if err := scanner.Err(); err != nil {
		fmt.Println("Erreur de lecture:", err)
	}
	fmt.Println("Client déconnecté:", conn.RemoteAddr())
}
	