#!/bin/bash

ERIC_PROJEKTDIR=$( pwd )
ERIC_DEMO=$ERIC_PROJEKTDIR/target/ericdemo.jar
JAVA_PROG="java"

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

if [ -e "$ERIC_DEMO" ]; then
    echo "Starte ericdemo ..."
    $JAVA_PROG -jar "$ERIC_DEMO" $@
else
    echo "Programm nicht gefunden: $JAVA_PROG $ERIC_DEMO"
    echo "Bitte erstellen Sie das Programm durch den Aufruf von 'ant' oder 'mvn package'"
    echo "Bitte Taste druecken"
    read -sn 1 ack
    exit 1
fi
