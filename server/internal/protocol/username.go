package protocol

import (
	"fmt"
	"unicode"
	"unicode/utf8"
)

const MaxUsernameLength = 32

// ValidateUsername: 1-32 Unicode letters, digits or '_' (RFC 9.2).
func ValidateUsername(username string) error {
	if username == "" {
		return fmt.Errorf("username cannot be empty")
	}

	if utf8.RuneCountInString(username) > MaxUsernameLength {
		return fmt.Errorf("username cannot exceed %d characters", MaxUsernameLength)
	}

	for _, char := range username {
		if !unicode.IsLetter(char) && !unicode.IsDigit(char) && char != '_' {
			return fmt.Errorf("username contains invalid character: %q", char)
		}
	}

	return nil
}
