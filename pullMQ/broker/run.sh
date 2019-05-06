#!/bin/sh
export BROKER_HOST="127.0.0.1"
make clean
make
./broker 12331