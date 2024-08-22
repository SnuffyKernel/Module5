#!bin/bash

sleep 2

for ((i=1; i<=7; i++)); do
    sleep 3
    echo $i | sudo tee /sys/kernel/kbleds/test
done