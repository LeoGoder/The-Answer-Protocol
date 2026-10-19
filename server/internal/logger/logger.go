// Package logger writes JSON lines: INFO/WARN to stdout, ERROR to stderr.
package logger

import (
	"bytes"
	"encoding/json"
	"io"
	"os"
	"sort"
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
	_, _ = out.Write(buffer.Bytes())
}

func writeValue(buffer *bytes.Buffer, value interface{}) {
	data, err := json.Marshal(value)
	if err != nil {
		data, _ = json.Marshal(err.Error())
	}
	buffer.Write(data)
}
