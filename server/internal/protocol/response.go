package protocol

const (
	OKHello     = "OK hello proto=1"
	OKConnected = "OK connected"
	OKBye       = "OK bye"
	OK          = "OK"
)

// Errors defined by the RFC (section 8.2).
const (
	ErrNameInUse        = "ERR 201 NAME_IN_USE"
	ErrNoExit           = "ERR 301 NO_EXIT"
	ErrNotInGroup       = "ERR 401 NOT_IN_GROUP"
	ErrAlreadyInGroup   = "ERR 402 ALREADY_IN_GROUP"
	ErrItemNotFound     = "ERR 404 ITEM_NOT_FOUND"
	ErrItemNotInInv     = "ERR 404 ITEM_NOT_IN_INVENTORY"
	ErrNPCNotFound      = "ERR 404 NPC_NOT_FOUND"
	ErrNPCNotHostile    = "ERR 405 NPC_NOT_HOSTILE"
	ErrNoQuestAvailable = "ERR 406 NO_QUEST_AVAILABLE"
	ErrConnectionFailed = "ERR 900 CONNECTION_FAILED"
)

// Errors added by this server (documented in the README).
const (
	ErrNotAuthenticated = "ERR 202 NOT_AUTHENTICATED"
	ErrBadRequest       = "ERR 400 BAD_REQUEST"
	ErrUnknownCommand   = "ERR 400 UNKNOWN_COMMAND"
	ErrLineTooLong      = "ERR 400 LINE_TOO_LONG"
	ErrPlayerNotFound   = "ERR 404 PLAYER_NOT_FOUND"
	ErrNotInvited       = "ERR 403 NOT_INVITED"
	ErrItemNotUsable    = "ERR 408 ITEM_NOT_USABLE"
	ErrInternal         = "ERR 500 INTERNAL_ERROR"
)
