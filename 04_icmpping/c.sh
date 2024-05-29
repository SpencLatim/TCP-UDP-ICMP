#!/bin/bash
../utils/ns_create.sh
echo "Note: We are sleeping for 3 seconds to wait for space2 to be ready..."
sleep 3
gcc icmp_ping_client.c -o icmp_ping_client;./icmp_ping_client
../utils/ns_rem.sh