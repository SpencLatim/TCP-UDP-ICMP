#!/bin/bash
../utils/ns_create.sh
../utils/ns_use.sh "ping 192.168.56.40 -i 0.2"
../utils/ns_rem.sh