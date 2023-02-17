#!/usr/bin/env bash

trace=$1
data_dir="data_final_pass"
mkdir -p $data_dir/$trace/

while LINE= read -r line; do
    c=$(echo $line | cut -d ' ' -f 1)
    b=$(echo $line | cut -d ' ' -f 2)
    s=$(echo $line | cut -d ' ' -f 3)
    p=$(echo $line | cut -d ' ' -f 4)
    t=$(echo $line | cut -d ' ' -f 5)
    i=0
    hlimit=$((32-p<20 ? 32-p : 20))
    for m in $(seq $p $hlimit); do
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
done < top_50_configs.txt
