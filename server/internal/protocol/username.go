package protocol

import (
	"fmt"
	"strings"
	"unicode/utf8"
)

const MaxUsernameLength = 32

// Add new allowed special characters here later.
var AllowedUsernameSpecialChars = []rune{
	'_',
}

// Add forbidden words here after team discussion.
// Matching is case-insensitive.
var ForbiddenUsernameParts = []string{
	"admin",
	"administrator",
}

func ValidateUsername(username string) error {
	if username == "" {
		return fmt.Errorf("username cannot be empty")
	}

	if utf8.RuneCountInString(username) > MaxUsernameLength {
		return fmt.Errorf(
			"username cannot exceed %d characters",
			MaxUsernameLength,
		)
	}

	for _, char := range username {
		if !isAlphaNumeric(char) &&
			!isAllowedSpecialChar(char) {
			return fmt.Errorf(
				"username contains invalid character: %q",
				char,
			)
		}
	}

	if hasTooManyRepeatedCharacters(username) {
		return fmt.Errorf(
			"username cannot contain more than 3 identical consecutive characters",
		)
	}

	lowerUsername := strings.ToLower(username)

	for _, forbidden := range ForbiddenUsernameParts {
		if strings.Contains(
			lowerUsername,
			strings.ToLower(forbidden),
		) {
			return fmt.Errorf(
				"username contains forbidden word: %s",
				forbidden,
			)
		}
	}

	return nil
}

func isAlphaNumeric(char rune) bool {
	return (char >= 'a' && char <= 'z') ||
		(char >= 'A' && char <= 'Z') ||
		(char >= '0' && char <= '9')
}

func isAllowedSpecialChar(char rune) bool {
	for _, allowed := range AllowedUsernameSpecialChars {
		if char == allowed {
			return true
		}
	}

	return false
}

func hasTooManyRepeatedCharacters(username string) bool {
	var previous rune
	repeatCount := 0

	for _, char := range username {
		if char == previous {
			repeatCount++
		} else {
			previous = char
			repeatCount = 1
		}

		if repeatCount > 3 {
			return true
		}
	}

	return false
}