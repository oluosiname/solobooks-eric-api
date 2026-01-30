#!/bin/bash

ERIC_PROJEKTDIR=$( pwd )
ERIC_DEMO=$ERIC_PROJEKTDIR/ericdemo/Debug/ericdemo

if [ "`uname -s`" = "AIX" ]; then
    export LDR_PRELOAD64="/usr/lib/libc++.a(shr_64.o)"

    while getopts v:x:c:p:d:l:s:t:neh? opt 2>/dev/null
    do
        case $opt in
            d) APIPATH="$OPTARG";;
        esac
    done

    if [ -z "$APIPATH" ]; then
        APIPATH=`cd ${ERIC_PROJEKTDIR}/../../lib && pwd`
    fi

    if [ ! -f "${APIPATH}/libericapi.so" ]; then
        echo "libericapi.so konnte in ${APIPATH} nicht gefunden werden"
        exit 1
    fi

    if [ -z "$LIBPATH" ]; then
        export LIBPATH=$APIPATH
    else
        export LIBPATH=$APIPATH:$LIBPATH
    fi
fi

if [ -x "$ERIC_DEMO" ]; then
    echo Starte ericdemo ...
    "$ERIC_DEMO" $@
    exit $?
else
    echo "Programm nicht gefunden: $ERIC_DEMO"
    echo "Bitte erstellen Sie ericdemo unter Linux durch den Aufruf von 'make', unter AIX durch den Aufruf von build-AIX.sh und unter macOS mit XCode"
    echo "Bitte Taste druecken"
    read -sn 1 ack
    exit 1
fi
