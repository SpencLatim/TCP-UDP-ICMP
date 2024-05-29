#!/bin/bash
sudo ip netns add space2
sudo ip link set enp0s8 netns space2
sudo ip netns exec space2 ip addr add dev enp0s8 192.168.56.80/24
sudo ip netns exec space2 ip link set dev enp0s8 up

