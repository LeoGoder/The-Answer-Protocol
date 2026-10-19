package protocol

import (
	"fmt"
	"strings"
	"unicode"
	"unicode/utf8"
)

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
