package protocol

import "strings"

func Parse(line string) Command {
	line = strings.TrimSpace(line)
	parts := strings.Fields(line)

	if len(parts) == 0 {
		return Command{}
	}

	return Command{
		Name: strings.ToUpper(parts[0]),
		Args: parts[1:],
	}
}
