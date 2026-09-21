package protocol

const (
	OKHello     = "OK hello proto=1"
	OKConnected = "OK connected"
	OKBye       = "OK bye"

	ErrNameInUse        = "ERR 201 NAME_IN_USE"
	ErrNotAuthenticated = "ERR 202 NOT_AUTHENTICATED"
	ErrBadRequest       = "ERR 400 BAD_REQUEST"
	ErrUnknownCommand   = "ERR 400 UNKNOWN_COMMAND"
)
