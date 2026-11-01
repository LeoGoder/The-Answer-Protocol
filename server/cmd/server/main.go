package main

import (
	"flag"
	"net"
	"os"
	"sync/atomic"
	"time"

	"tap/server/internal/game"
	"tap/server/internal/logger"
	"tap/server/internal/protocol"
)

const MaxClients = 64

// Shared by all goroutines: only changed with atomic operations.
var activeClients int32

func main() {
	worldPath := flag.String("world", "data/world.json", "path to the world JSON file")
	port := flag.String("port", "4242", "TCP port to listen on")
	flag.Parse()

	logger.Info("server_starting", logger.Fields{"world": *worldPath, "port": *port})

	state, err := game.LoadState(*worldPath)
	if err != nil {
		logger.Error("world_load_failed", logger.Fields{"error": err.Error()})
		os.Exit(1)
	}

	logger.Info("world_loaded", logger.Fields{"world": *worldPath})

	listener, err := net.Listen("tcp", ":"+*port)
	if err != nil {
		logger.Error("listen_failed", logger.Fields{"port": *port, "error": err.Error()})
		os.Exit(1)
	}
	defer listener.Close()

	logger.Info("server_listening", logger.Fields{"port": *port})

	hub := NewClientHub()
	monitor := newConnectionMonitor()

	go broadcastPositions(state, hub)
	go tickItems(state, hub)

	for {
		conn, err := listener.Accept()
		if err != nil {
			logger.Warn("accept_failed", logger.Fields{"error": err.Error()})
			continue
		}

		monitor.record(conn.RemoteAddr())

		if atomic.AddInt32(&activeClients, 1) > MaxClients {
			atomic.AddInt32(&activeClients, -1)
			logger.Warn("connection_refused", logger.Fields{
				"ip": conn.RemoteAddr().String(), "reason": "server_full",
			})
			send(conn, "", protocol.ErrConnectionFailed)
			conn.Close()
			continue
		}

		go handleClient(conn, state, hub)
	}
}

// tickItems expires dropped items and refills spawn points, once per second.
func tickItems(state *game.State, hub *ClientHub) {
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()

	for range ticker.C {
		hub.Deliver(state.Tick())
	}
}

const PositionRate = 10

func broadcastPositions(state *game.State, hub *ClientHub) {
	ticker := time.NewTicker(time.Second / PositionRate)
	defer ticker.Stop()

	for range ticker.C {
		hub.Deliver(state.PositionEvents())
	}
}
