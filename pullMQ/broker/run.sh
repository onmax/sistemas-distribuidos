#!/bin/sh
export BROKER_HOST="localhost"
make clean
make
./broker 12353