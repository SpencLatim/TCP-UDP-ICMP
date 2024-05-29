#!/bin/bash
../utils/ns_create.sh
gcc client2.c -o client2
../utils/ns_use.sh ./client2
../utils/ns_rem.sh