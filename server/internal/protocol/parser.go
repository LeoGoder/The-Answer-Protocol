package protocol

import (
	"fmt"
	"strings"
	"unicode"
	"unicode/utf8"
)

type Command struct {
	Name string
	Args []string
	Raw  string // the line as received, without "\r\n" (chat keeps its own spacing)
}

const MaxLineLength = 1024

// CheckLine rejects invalid UTF-8 and control characters (RFC 9.2).
func CheckLine(line string) error {
	line = strings.TrimRight(line, "\r\n")

	if !utf8.ValidString(line) {
		return fmt.Errorf("line is not valid UTF-8")
	}

	for _, char := range line {
		if char != '\t' && unicode.IsControl(char) {
			return fmt.Errorf("line contains control character %U", char)
		}
	}

	return nil
}

func Parse(line string) Command {
	raw := strings.TrimRight(line, "\r\n")
	parts := strings.Fields(raw)

	if len(parts) == 0 {
		return Command{}
	}

	return Command{
		Name: strings.ToUpper(parts[0]),
		Args: parts[1:],
		Raw:  raw,
	}
}
