#!/usr/bin/env bash

trace=$1
data_dir="data_first_pass"
mkdir -p $data_dir/$trace/

i=0
for c in {9..15}; do
    for b in {4..7}; do
        p=$((c<14 ? c : 14))
        hlimit=$((c-b))
        llimit=$((c-p))
        for s in $llimit $hlimit; do
            t=$((c-b-s))
            m=$((32-p<20 ? 32-p : 20))

            echo "Configurations Run: $i"
            output=$(./cachesim -v -c $c -b $b -p $p -s $s -t $t -m $m < ./traces/${trace}.trace)
            data=$(cat <<< $output | grep -P -o ': .*' | cut -d ' ' -f 2)
            amat=$(cat <<< $data | tail -n 1)
            echo >> $data_dir/$trace/all_data.txt
            echo "$c $b $s $p $t $m" >> $data_dir/$trace/all_data.txt
            cat <<< $data >> $data_dir/$trace/all_data.txt
            echo $amat >> $data_dir/$trace/access_times.txt
            echo "$c $b $s $p $t $m" >> $data_dir/$trace/configs.txt
            i=$((i+1))
        done;
    done;
done;
