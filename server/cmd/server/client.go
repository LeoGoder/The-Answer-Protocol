package main

import (
	"net"
	"sync"
)

type ClientHub struct {
	mutex   sync.Mutex
	clients map[string]net.Conn
}

func NewClientHub() *ClientHub {
	return &ClientHub{
		clients: make(map[string]net.Conn),
	}
}

func (h *ClientHub) Add(username string, conn net.Conn) {
	h.mutex.Lock()
	defer h.mutex.Unlock()

	h.clients[username] = conn
}

func (h *ClientHub) Remove(username string) {
	h.mutex.Lock()
	defer h.mutex.Unlock()

	delete(h.clients, username)
}

func (h *ClientHub) SendTo(username, message string) {
	h.mutex.Lock()
	conn, exists := h.clients[username]
	h.mutex.Unlock()

	if !exists {
		return
	}

	send(conn, message)
}

func (h *ClientHub) SendToAll(message string) {
	h.mutex.Lock()

	connections := make([]net.Conn, 0, len(h.clients))
	for _, conn := range h.clients {
		connections = append(connections, conn)
	}

	h.mutex.Unlock()

	for _, conn := range connections {
		send(conn, message)
	}
}
