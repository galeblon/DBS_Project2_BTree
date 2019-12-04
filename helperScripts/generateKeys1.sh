#!/bin/bash
for i in $(seq $1)
do
    echo i $i $(($RANDOM%100)) $(($RANDOM%100)) $(($RANDOM%100))
done
