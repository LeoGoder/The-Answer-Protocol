package protocol

import (
	"fmt"
	"strings"
	"unicode"
	"unicode/utf8"
)

const MaxUsernameLength = 32

// ValidateUsername: 1 to 32 Unicode letters, digits or '_' (RFC 9.2),
// at most 3 identical characters in a row, and no "admin" inside.
func ValidateUsername(username string) error {
	if username == "" {
		return fmt.Errorf("username cannot be empty")
	}

	if utf8.RuneCountInString(username) > MaxUsernameLength {
		return fmt.Errorf("username cannot exceed %d characters", MaxUsernameLength)
	}

	var previous rune
	repeated := 0

	for _, char := range username {
		if !unicode.IsLetter(char) && !unicode.IsDigit(char) && char != '_' {
			return fmt.Errorf("username contains invalid character: %q", char)
		}

		if char == previous {
			repeated++
		} else {
			previous = char
			repeated = 1
		}

		if repeated > 3 {
			return fmt.Errorf("username cannot contain more than 3 identical consecutive characters")
		}
	}

	if strings.Contains(strings.ToLower(username), "admin") {
		return fmt.Errorf("username contains forbidden word: admin")
	}

	return nil
}
