package main

import (
	"net"
	"sync"
	"time"

	"tap/server/internal/game"
	"tap/server/internal/logger"
)

// A client that stops reading cannot block a sender longer than this.
const WriteTimeout = 2 * time.Second

// ClientHub maps usernames to connections.
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

// Deliver goes on if a recipient fails; its own goroutine cleans it up.
func (h *ClientHub) Deliver(events []game.Event) {
	for _, event := range events {
		for _, username := range event.To {
			h.mutex.Lock()
			conn, exists := h.clients[username]
			h.mutex.Unlock()

			// Send outside the lock: a slow client must not block the hub.
			if exists {
				send(conn, username, event.Line)
			}
		}
	}
}

func send(conn net.Conn, username string, message string) bool {
	_ = conn.SetWriteDeadline(time.Now().Add(WriteTimeout))

	_, err := conn.Write([]byte(message + "\n"))
	if err != nil {
		logger.Error("send_failed", logger.Fields{
			"ip": conn.RemoteAddr().String(), "player": username, "error": err.Error(),
		})
		return false
	}

	logger.Info("message_sent", logger.Fields{
		"ip": conn.RemoteAddr().String(), "player": username, "message": message,
	})

	return true
}
