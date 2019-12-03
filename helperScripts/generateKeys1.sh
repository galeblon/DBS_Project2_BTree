#!/bin/bash
for i in {1..50}
do
    echo i $((50 -$i)) $(($RANDOM%100)) $(($RANDOM%100)) $(($RANDOM%100))
    echo i $((50 +$i)) $(($RANDOM%100)) $(($RANDOM%100)) $(($RANDOM%100))
done
