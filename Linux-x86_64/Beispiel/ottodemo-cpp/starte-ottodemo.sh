#!/bin/bash

OTTO_PROJEKT_DIR="$( pwd )"


if [ "$(uname -s)" = "AIX" ]; then

    OTTO_DEMO_BIN="$( find "$OTTO_PROJEKT_DIR"  -type f -name ottodemo -print | head -n 1)"

    while getopts e:s:c:p:o:d:l:i:h? opt 2>/dev/null
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
    OTTO_DEMO_BIN="$( find "$OTTO_PROJEKT_DIR"  -type f -maxdepth 3 -name ottodemo -print -quit)"
fi

if [ -x "$OTTO_DEMO_BIN" ]; then
    echo "Starte ottodemo ..."
    "$OTTO_DEMO_BIN" "$@"
    exit $?
else
    echo "Programm nicht gefunden: ottodemo"
    echo "Bitte erstellen Sie ottodemo unter Linux durch den Aufruf von 'make', unter AIX durch den Aufruf von build-AIX.sh und unter macOS mit XCode"
    echo "Bitte Taste drücken"
    read -sn 1 ack
    exit 1
fi
