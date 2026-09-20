package server

import (
	"net"
)

func Send(conn net.Conn, message string) error {
	_, err := conn.Write([]byte(message + "\n"))
	return err
}