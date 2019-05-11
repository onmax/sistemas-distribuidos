#!/bin/sh
export BROKER_HOST="onmax"
make clean
make
./broker 12391