package main

import (
	"bufio"
	"flag"
	"net"
	"os"
	"sync/atomic"

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

func handleClient(conn net.Conn, state *game.State, hub *ClientHub) {
	ip := conn.RemoteAddr().String()
	username := "" // stays empty until a successful CONNECT

	// Cleanup, whatever ends the connection.
	defer func() {
		if username != "" {
			hub.Remove(username)
			hub.Deliver(state.Leave(username))
			logger.Info("player_disconnected", logger.Fields{"player": username, "ip": ip})
		} else {
			logger.Info("client_disconnected", logger.Fields{"ip": ip})
		}

		conn.Close()
		atomic.AddInt32(&activeClients, -1)
	}()

	logger.Info("client_connected", logger.Fields{"ip": ip})

	if !send(conn, "", protocol.OKHello) {
		return
	}

	reader := bufio.NewReaderSize(conn, protocol.MaxLineLength)
	flood := &floodDetector{}

	for {
		line, tooLong, err := readLine(reader)
		if err != nil {
			logger.Info("connection_closed", logger.Fields{
				"player": username, "ip": ip, "reason": err.Error(),
			})
			return
		}

		flood.record(ip, username)

		if tooLong {
			logger.Warn("line_too_long", logger.Fields{"player": username, "ip": ip})

			if !send(conn, username, protocol.ErrLineTooLong) {
				return
			}

			continue
		}

		if err := protocol.CheckLine(line); err != nil {
			logger.Warn("invalid_line", logger.Fields{
				"player": username, "ip": ip, "error": err.Error(),
			})

			if !send(conn, username, protocol.ErrBadRequest) {
				return
			}

			continue
		}

		cmd := protocol.Parse(line)
		if cmd.Name == "" {
			continue // empty line: ignored silently
		}

		logger.Info("command_received", logger.Fields{
			"player": username, "ip": ip, "command": cmd.Name, "args": cmd.Args,
		})

		if err := protocol.Validate(cmd); err != nil {
			logger.Warn("invalid_command", logger.Fields{
				"player": username, "ip": ip, "command": cmd.Name, "error": err.Error(),
			})

			if !send(conn, username, protocol.ErrBadRequest) {
				return
			}

			continue
		}

		switch {
		case cmd.Name == "QUIT":
			send(conn, username, protocol.OKBye)
			return

		case cmd.Name == "CONNECT":
			if username != "" {
				logger.Warn("connect_rejected", logger.Fields{
					"player": username, "ip": ip, "reason": "already_authenticated",
				})

				if !send(conn, username, protocol.ErrBadRequest) {
					return
				}

				continue
			}

			events, ok := state.Join(cmd.Args[0])
			if !ok {
				logger.Warn("connect_rejected", logger.Fields{
					"player": cmd.Args[0], "ip": ip, "reason": "name_in_use",
				})

				if !send(conn, "", protocol.ErrNameInUse) {
					return
				}

				continue
			}

			username = cmd.Args[0]
			hub.Add(username, conn)

			logger.Info("player_authenticated", logger.Fields{"player": username, "ip": ip})

			if !send(conn, username, protocol.OKConnected) {
				return
			}

			hub.Deliver(events)

		// Single authentication check for every other command.
		case username == "":
			logger.Warn("unauthenticated_command", logger.Fields{"ip": ip, "command": cmd.Name})

			if !send(conn, "", protocol.ErrNotAuthenticated) {
				return
			}

		default:
			result := state.Handle(username, cmd)

			if !send(conn, username, result.Reply) {
				return
			}

			hub.Deliver(result.Events)
		}
	}
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
