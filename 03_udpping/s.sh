#!/bin/bash
../utils/ns_create.sh
gcc ping_server.c -o ping_server;
../utils/ns_use.sh ./ping_server
../utils/ns_rem.sh