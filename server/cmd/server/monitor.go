package main

import (
	"net"
	"time"

	"tap/server/internal/logger"
)

const (
	FloodLimit  = 20
	FloodWindow = time.Second

	RapidLimit  = 5
	RapidWindow = 10 * time.Second
)

// Used by one client goroutine only: no mutex needed.
type floodDetector struct {
	windowStart time.Time
	count       int
	reported    bool
}

func (f *floodDetector) record(ip string, username string) {
	now := time.Now()

	if now.Sub(f.windowStart) > FloodWindow {
		f.windowStart = now
		f.count = 0
		f.reported = false
	}

	f.count++

	if f.count > FloodLimit && !f.reported {
		f.reported = true
		logger.Warn("command_flood", logger.Fields{
			"ip": ip, "player": username, "commands": f.count,
			"window_ms": FloodWindow.Milliseconds(),
		})
	}
}

// Used by the accept loop only: no mutex needed.
type connectionMonitor struct {
	recent map[string][]time.Time
}

func newConnectionMonitor() *connectionMonitor {
	return &connectionMonitor{recent: make(map[string][]time.Time)}
}

func (m *connectionMonitor) record(address net.Addr) {
	host, _, err := net.SplitHostPort(address.String())
	if err != nil {
		host = address.String()
	}

	now := time.Now()

	kept := make([]time.Time, 0, len(m.recent[host])+1)
	for _, moment := range m.recent[host] {
		if now.Sub(moment) <= RapidWindow {
			kept = append(kept, moment)
		}
	}

	kept = append(kept, now)
	m.recent[host] = kept

	if len(kept) > RapidLimit {
		logger.Warn("rapid_connections", logger.Fields{
			"ip": host, "connections": len(kept),
			"window_seconds": int(RapidWindow.Seconds()),
		})
	}
}
