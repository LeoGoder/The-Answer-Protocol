// Package logger writes JSON lines: INFO/WARN to stdout, ERROR to stderr.
package logger

import (
	"bytes"
	"encoding/json"
	"io"
	"os"
	"sort"
	"strings"
	"sync"
	"time"
)

type Fields map[string]interface{}

var mutex sync.Mutex

func Info(event string, fields Fields) {
	write(os.Stdout, "INFO", event, fields)
}

func Warn(event string, fields Fields) {
	write(os.Stdout, "WARN", event, fields)
}

func Error(event string, fields Fields) {
	write(os.Stderr, "ERROR", event, fields)
}

func colorFor(level string, event string, fields Fields) string {
	if level == "ERROR" {
		return "\033[31m"
	}
	if event == "command_received" {
		return "\033[33m"
	}
	if message, ok := fields["message"].(string); ok {
		switch {
		case strings.HasPrefix(message, "OK"):
			return "\033[32m"
		case strings.HasPrefix(message, "ERR"):
			return "\033[31m"
		case strings.HasPrefix(message, "EVT"):
			return "\033[2m"
		}
	}
	return ""
}

func isTerminal(file *os.File) bool {
	info, err := file.Stat()
	return err == nil && info.Mode()&os.ModeCharDevice != 0
}

// "time", "level" and "event" first, then the fields sorted by key.
func write(out io.Writer, level string, event string, fields Fields) {
	var buffer bytes.Buffer

	buffer.WriteString(`{"time":`)
	writeValue(&buffer, time.Now().UTC().Format(time.RFC3339Nano))
	buffer.WriteString(`,"level":`)
	writeValue(&buffer, level)
	buffer.WriteString(`,"event":`)
	writeValue(&buffer, event)

	keys := make([]string, 0, len(fields))
	for key := range fields {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	for _, key := range keys {
		buffer.WriteString(",")
		writeValue(&buffer, key)
		buffer.WriteString(":")
		writeValue(&buffer, fields[key])
	}

	buffer.WriteString("}\n")

	mutex.Lock()
	defer mutex.Unlock()
	line := buffer.Bytes()
	if file, ok := out.(*os.File); ok && isTerminal(file) {
		if color := colorFor(level, event, fields); color != "" {
			line = []byte(color + strings.TrimSuffix(string(line), "\n") + "\033[0m\n")
		}
	}
	_, _ = out.Write(line)
	//_, _ = out.Write(buffer.Bytes())
}

func writeValue(buffer *bytes.Buffer, value interface{}) {
	data, err := json.Marshal(value)
	if err != nil {
		data, _ = json.Marshal(err.Error())
	}
	buffer.Write(data)
}
