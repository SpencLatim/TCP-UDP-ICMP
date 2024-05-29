#!/bin/bash
../utils/ns_create.sh
gcc server.c -o server;
../utils/ns_use.sh ./server
../utils/ns_rem.sh