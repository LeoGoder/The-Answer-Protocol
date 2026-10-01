package main

import (
	"log"
	"os"
)

var infoLogger = log.New(
	os.Stdout,
	"INFO ",
	log.Ldate|log.Ltime,
)

var warnLogger = log.New(
	os.Stdout,
	"WARN ",
	log.Ldate|log.Ltime,
)

var errorLogger = log.New(
	os.Stderr,
	"ERROR ",
	log.Ldate|log.Ltime,
)

func logInfo(format string, args ...interface{}) {
	infoLogger.Printf(format, args...)
}

func logWarn(format string, args ...interface{}) {
	warnLogger.Printf(format, args...)
}

func logError(format string, args ...interface{}) {
	errorLogger.Printf(format, args...)
}
