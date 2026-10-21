#!/bin/bash

ERIC_PROJEKT_DIR="$( pwd )"


if [ "$(uname -s)" = "AIX" ]; then

    ERIC_DEMO_BIN="$( find "$ERIC_PROJEKT_DIR"  -type f  -name ericdemo -print | head -n 1)"

    while getopts v:x:c:p:d:l:s:t:neh? opt 2>/dev/null
    do
        case $opt in
            d) APIPATH="$OPTARG";;
        esac
    done

    if [ -n "$APIPATH" ] && [ -z "$LIBPATH" ]; then
	      export LIBPATH="$APIPATH"
    elif [ -n "$APIPATH" ]; then
        export LIBPATH="$APIPATH:$LIBPATH"
    fi
else
    ERIC_DEMO_BIN="$( find "$ERIC_PROJEKT_DIR"  -type f  -maxdepth 3  -name ericdemo -print -quit)"
fi

if [ -x "$ERIC_DEMO_BIN" ]; then
    echo "Starte ericdemo ..."
    "$ERIC_DEMO_BIN" "$@"
    exit $?
else
    echo "Programm nicht gefunden: ericdemo"
    echo "Bitte erstellen Sie ericdemo unter Linux durch den Aufruf von 'make', unter AIX durch den Aufruf von build-AIX.sh und unter macOS mit XCode"
    echo "Bitte Taste druecken"
    read -sn 1 ack
    exit 1
fi
