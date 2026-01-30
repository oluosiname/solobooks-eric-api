#!/bin/bash

OTTO_PROJEKTDIR=$( pwd )
OTTO_DEMO=$OTTO_PROJEKTDIR/target/ottodemo.jar
JAVA_PROG="java"

if [ "`uname -s`" = "AIX" ]; then
    export LDR_PRELOAD64="/usr/lib/libc++.a(shr_64.o)"

    while getopts e:s:c:p:o:d:l:i:h? opt 2>/dev/null
    do
        case $opt in
            d) APIPATH="$OPTARG";;
        esac
    done

    if [ -z "$APIPATH" ]; then
        APIPATH=`cd ${OTTO_PROJEKTDIR}/../../lib && pwd`
    fi

    if [ ! -f "${APIPATH}/libotto.so" ]; then
        echo "libotto.so konnte in ${APIPATH} nicht gefunden werden"
        exit 1
    fi

    if [ -z "$LIBPATH" ]; then
        export LIBPATH=$APIPATH
    else
        export LIBPATH=$APIPATH:$LIBPATH
    fi
fi

if [ -e "$OTTO_DEMO" ]; then
    echo "Starte ottodemo ..."
    $JAVA_PROG -jar "$OTTO_DEMO" $@
else
    echo "Programm nicht gefunden: $JAVA_PROG $OTTO_DEMO"
    echo "Bitte erstellen Sie das Programm durch den Aufruf von 'ant' oder 'mvn package"
    echo "Bitte Taste druecken"
    read -sn 1 ack
    exit 1
fi
