#!/bin/bash -xe

time python3 bench.py
time node bench.js
time ../../swarmc --locally --svi bench.svi

