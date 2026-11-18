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
	Raw  string // the line as received, without "\r\n"
}

// After returns the raw text that follows the first n words of the line (the command
// name counts), without the one space or tab right after them: a chat message keeps
// its own spacing, while Args are split on any run of spaces.
func (c Command) After(n int) string {
	text := c.Raw

	for i := 0; i < n; i++ {
		text = strings.TrimLeft(text, " \t")

		end := strings.IndexAny(text, " \t")
		if end < 0 {
			return ""
		}

		text = text[end:]
	}

	return text[1:]
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
