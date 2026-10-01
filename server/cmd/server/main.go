package main

import (
	"bufio"
	"fmt"
	"net"
	"strings"

	"tap/server/internal/game"
	"tap/server/internal/protocol"
)

func send(conn net.Conn, message string) bool {
	_, err := conn.Write([]byte(message + "\n"))
	if err != nil {
		logError(
			"send_failed ip=%s error=%v",
			conn.RemoteAddr(),
			err,
		)
		return false
	}

	logInfo(
		"message_sent ip=%s message=%q",
		conn.RemoteAddr(),
		message,
	)

	return true
}

func sendPresenceEvent(
	state *game.State,
	hub *ClientHub,
	roomID string,
	username string,
	eventType string,
) {
	event := fmt.Sprintf(
		"EVT ROOM PRESENCE %s %s",
		eventType,
		username,
	)

	players := state.PlayersInRoom(roomID)

	for _, player := range players {
		if player == username {
			continue
		}

		hub.SendTo(player, event)
	}
}

func handleClient(
	conn net.Conn,
	state *game.State,
	hub *ClientHub,
) {
	username := ""

	defer func() {
		if username != "" {
			roomID, exists := state.PlayerRoom(username)

			hub.Remove(username)
			state.RemovePlayer(username)

			if exists {
				sendPresenceEvent(
					state,
					hub,
					roomID,
					username,
					"LEAVE",
				)
			}

			logInfo(
				"player_disconnected player=%s ip=%s",
				username,
				conn.RemoteAddr(),
			)
		} else {
			logInfo(
				"client_disconnected ip=%s",
				conn.RemoteAddr(),
			)
		}

		conn.Close()
	}()

	logInfo(
		"client_connected ip=%s",
		conn.RemoteAddr(),
	)

	if !send(conn, protocol.OKHello) {
		return
	}

	reader := bufio.NewReader(conn)

	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			logInfo(
				"connection_closed player=%q ip=%s",
				username,
				conn.RemoteAddr(),
			)
			return
		}

		cmd := protocol.Parse(line)

		if cmd.Name == "" {
			continue
		}

		if err := protocol.Validate(cmd); err != nil {
			logWarn(
				"invalid_command player=%q ip=%s command=%s args=%v error=%v",
				username,
				conn.RemoteAddr(),
				cmd.Name,
				cmd.Args,
				err,
			)

			if !send(conn, protocol.ErrBadRequest) {
				return
			}

			continue
		}

		logInfo(
			"command player=%q ip=%s command=%s args=%v",
			username,
			conn.RemoteAddr(),
			cmd.Name,
			cmd.Args,
		)

		switch cmd.Name {

		case "QUIT":
			send(conn, protocol.OKBye)
			return

		case "CONNECT":
			if username != "" {
				logWarn(
					"connect_rejected player=%s reason=already_authenticated",
					username,
				)

				if !send(conn, protocol.ErrBadRequest) {
					return
				}

				continue
			}

			requested := cmd.Args[0]

			if !state.AddPlayer(requested) {
				logWarn(
					"connect_rejected player=%s reason=name_in_use",
					requested,
				)

				if !send(conn, protocol.ErrNameInUse) {
					return
				}

				continue
			}

			username = requested

			hub.Add(username, conn)

			logInfo(
				"player_authenticated player=%s ip=%s",
				username,
				conn.RemoteAddr(),
			)

			if !send(conn, protocol.OKConnected) {
				return
			}

			roomID, exists := state.PlayerRoom(username)
			if exists {
				sendPresenceEvent(
					state,
					hub,
					roomID,
					username,
					"ENTER",
				)
			}

		case "CHAT":
			if username == "" {
				logWarn(
					"unauthenticated_command ip=%s command=CHAT",
					conn.RemoteAddr(),
				)

				if !send(
					conn,
					protocol.ErrNotAuthenticated,
				) {
					return
				}

				continue
			}

			handleChat(
				conn,
				state,
				hub,
				username,
				cmd,
			)

		case "MOVE":
			if username == "" {
				logWarn(
					"unauthenticated_command ip=%s command=MOVE",
					conn.RemoteAddr(),
				)

				if !send(
					conn,
					protocol.ErrNotAuthenticated,
				) {
					return
				}

				continue
			}

			oldRoom, oldRoomExists := state.PlayerRoom(username)

			response := state.Handle(username, cmd)

			if !send(conn, response) {
				return
			}

			if !strings.HasPrefix(response, "OK room=") {
				continue
			}

			newRoom, newRoomExists := state.PlayerRoom(username)

			if oldRoomExists {
				sendPresenceEvent(
					state,
					hub,
					oldRoom,
					username,
					"LEAVE",
				)
			}

			if newRoomExists {
				sendPresenceEvent(
					state,
					hub,
					newRoom,
					username,
					"ENTER",
				)
			}

			logInfo(
				"player_moved player=%s from=%s to=%s",
				username,
				oldRoom,
				newRoom,
			)

		case "LOOK", "WHO", "GROUP",
			"TAKE", "DROP", "INVENTORY", "TALK",
			"ATTACK", "STATUS", "QUEST", "QUESTS":

			if username == "" {
				logWarn(
					"unauthenticated_command ip=%s command=%s",
					conn.RemoteAddr(),
					cmd.Name,
				)

				if !send(
					conn,
					protocol.ErrNotAuthenticated,
				) {
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
			logWarn(
				"unknown_command player=%q ip=%s command=%s",
				username,
				conn.RemoteAddr(),
				cmd.Name,
			)

			if !send(
				conn,
				protocol.ErrUnknownCommand,
			) {
				return
			}
		}
	}
}

func handleChat(
	conn net.Conn,
	state *game.State,
	hub *ClientHub,
	username string,
	cmd protocol.Command,
) {
	scope := strings.ToUpper(cmd.Args[0])

	message := strings.Join(
		cmd.Args[1:],
		" ",
	)

	logInfo(
		"chat player=%s scope=%s message=%q",
		username,
		scope,
		message,
	)

	if !send(conn, "OK") {
		return
	}

	switch scope {

	case "GLOBAL":
		event := fmt.Sprintf(
			"EVT GLOBAL CHAT %s %s",
			username,
			message,
		)

		hub.SendToAll(event)

	case "ROOM":
		event := fmt.Sprintf(
			"EVT ROOM CHAT %s %s",
			username,
			message,
		)

		players := state.PlayersInSameRoom(
			username,
		)

		for _, player := range players {
			hub.SendTo(player, event)
		}

	case "GROUP":
		// Group chat will be implemented
		// when the real group system is ready.
	}
}

func main() {
	logInfo("server_starting")

	state, err := game.LoadState()
	if err != nil {
		logError(
			"world_load_failed error=%v",
			err,
		)
		return
	}

	logInfo("world_loaded")

	hub := NewClientHub()

	listener, err := net.Listen("tcp", ":4242")
	if err != nil {
		logError(
			"listen_failed port=4242 error=%v",
			err,
		)
		return
	}

	defer listener.Close()

	logInfo("server_listening port=4242")

	for {
		conn, err := listener.Accept()
		if err != nil {
			logWarn(
				"accept_failed error=%v",
				err,
			)
			continue
		}

		go handleClient(
			conn,
			state,
			hub,
		)
	}
}
