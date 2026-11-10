package main

import (
	"bufio"
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

// client is one connection; username stays empty until a successful CONNECT.
type client struct {
	conn     net.Conn
	ip       string
	username string
	state    *game.State
	hub      *ClientHub
}

// handleClient runs for the whole life of one connection: greet, read lines, clean up.
func handleClient(conn net.Conn, state *game.State, hub *ClientHub) {
	c := &client{conn: conn, ip: conn.RemoteAddr().String(), state: state, hub: hub}

	// Cleanup, whatever ends the connection.
	defer func() {
		if c.username != "" {
			hub.Remove(c.username)
			hub.Deliver(state.Leave(c.username))
			logger.Info("player_disconnected", logger.Fields{"player": c.username, "ip": c.ip})
		} else {
			logger.Info("client_disconnected", logger.Fields{"ip": c.ip})
		}

		conn.Close()
		atomic.AddInt32(&activeClients, -1)
	}()

	logger.Info("client_connected", logger.Fields{"ip": c.ip})

	if !c.reply(protocol.OKHello) {
		return
	}

	reader := bufio.NewReaderSize(conn, protocol.MaxLineLength)
	flood := &floodDetector{}

	for {
		line, tooLong, err := readLine(reader)
		if err != nil {
			logger.Info("connection_closed", logger.Fields{
				"player": c.username, "ip": c.ip, "reason": err.Error(),
			})
			return
		}

		flood.record(c.ip, c.username)

		if !c.handleLine(line, tooLong) {
			return
		}
	}
}

// handleLine answers one line, checked from the outside in.
// It returns false when the connection must end (QUIT or a failed write).
func (c *client) handleLine(line string, tooLong bool) bool {
	if tooLong {
		logger.Warn("line_too_long", logger.Fields{"player": c.username, "ip": c.ip})
		return c.reply(protocol.ErrLineTooLong)
	}

	if err := protocol.CheckLine(line); err != nil {
		logger.Warn("invalid_line", logger.Fields{
			"player": c.username, "ip": c.ip, "error": err.Error(),
		})
		return c.reply(protocol.ErrBadRequest)
	}

	cmd := protocol.Parse(line)
	if cmd.Name == "" {
		return true // empty line: ignored silently
	}

	logger.Info("command_received", logger.Fields{
		"player": c.username, "ip": c.ip, "command": cmd.Name, "args": cmd.Args,
	})

	if err := protocol.Validate(cmd); err != nil {
		logger.Warn("invalid_command", logger.Fields{
			"player": c.username, "ip": c.ip, "command": cmd.Name, "error": err.Error(),
		})
		return c.reply(protocol.ErrBadRequest)
	}

	switch {
	case cmd.Name == "QUIT":
		c.reply(protocol.OKBye)
		return false

	case cmd.Name == "CONNECT":
		return c.connect(cmd.Args[0])

	// Single authentication check for every other command.
	case c.username == "":
		logger.Warn("unauthenticated_command", logger.Fields{"ip": c.ip, "command": cmd.Name})
		return c.reply(protocol.ErrNotAuthenticated)

	default:
		result := c.state.Handle(c.username, cmd)
		if !c.reply(result.Reply) {
			return false
		}

		c.hub.Deliver(result.Events)
		return true
	}
}

// connect handles CONNECT <name>: once per connection, and the name must be free.
func (c *client) connect(name string) bool {
	if c.username != "" {
		logger.Warn("connect_rejected", logger.Fields{
			"player": c.username, "ip": c.ip, "reason": "already_authenticated",
		})
		return c.reply(protocol.ErrBadRequest)
	}

	events, ok := c.state.Join(name)
	if !ok {
		logger.Warn("connect_rejected", logger.Fields{
			"player": name, "ip": c.ip, "reason": "name_in_use",
		})
		return c.reply(protocol.ErrNameInUse)
	}

	c.username = name
	c.hub.Add(name, c.conn)

	logger.Info("player_authenticated", logger.Fields{"player": name, "ip": c.ip})

	if !c.reply(protocol.OKConnected) {
		return false
	}

	c.hub.Deliver(events)
	return true
}

// reply sends one line to this client; false means the write failed.
func (c *client) reply(message string) bool {
	return send(c.conn, c.username, message)
}

// readLine returns one full line; ErrBufferFull means longer than MaxLineLength.
func readLine(reader *bufio.Reader) (line string, tooLong bool, err error) {
	data, err := reader.ReadSlice('\n')
	if err == nil {
		return string(data), false, nil
	}

	if err != bufio.ErrBufferFull {
		return "", false, err
	}

	for err == bufio.ErrBufferFull {
		_, err = reader.ReadSlice('\n')
	}

	if err != nil {
		return "", true, err
	}

	return "", true, nil
}

const PositionRate = 24

func broadcastPositions(state *game.State, hub *ClientHub) {
	ticker := time.NewTicker(time.Second / PositionRate)
	defer ticker.Stop()

	for range ticker.C {
		hub.Deliver(state.PositionEvents())
	}
}
