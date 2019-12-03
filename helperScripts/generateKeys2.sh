#!/bin/bash
for i in {1..100}
do
    echo i $i $(($RANDOM%100)) $(($RANDOM%100)) $(($RANDOM%100))
done
