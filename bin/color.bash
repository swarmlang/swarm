#!/bin/bash
set +x
function black(){
    echo -e -n "\x1B[30m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[30m $($2) \x1B[0m"
    fi
}
function red(){
    echo -e -n "\x1B[31m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[31m $($2) \x1B[0m"
    fi
}
function green(){
    echo -e -n "\x1B[32m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[32m $($2) \x1B[0m"
    fi
}
function yellow(){
    echo -e -n "\x1B[33m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[33m $($2) \x1B[0m"
    fi
}
function blue(){
    echo -e -n "\x1B[34m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[34m $($2) \x1B[0m"
    fi
}
function purple(){
    echo -e -n "\x1B[35m $1 \x1B[0m \c"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[35m $($2) \x1B[0m"
    fi
}
function cyan(){
    echo -e -n "\x1B[36m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[36m $($2) \x1B[0m"
    fi
}
function white(){

    echo -e -n "\x1B[37m $1 \x1B[0m"
    if [ ! -z "${2}" ]; then
    echo -e -n "\x1B[33m $($2) \x1B[0m"
    fi
}

THECOLOR="$1"
shift

"$THECOLOR" "$1"
shift
echo "$*"