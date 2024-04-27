#!/bin/bash
# SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
# SPDX-License-Identifier: MIT

NRUNS=10
CMD=../build/accft
if [ "$(id -u)" = "0" ]; then
    echo "Running as root, setting nice to -20 and selecting performance governor"
    echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
    CMD="nice -n -20 $CMD"
fi

run_for_dataset() {
    pattern=$1
    parser_type=$2
    tot_runs=$3
    out_dir=$4
    mkdir -p $out_dir
    for inst in $pattern; do
        base_inst_name=$(basename -- $inst)
        base_inst_name=${base_inst_name%.*}
        inst_out=$out_dir/$base_inst_name.out

        printf "%-16s " $base_inst_name
        init_seed=1
        if [ -f "$inst_out" ]; then
            init_seed=$(wc -l <$inst_out)
            for s in $(seq 1 $init_seed); do printf "."; done
            init_seed=$((init_seed + 1))
        fi

        for s in $(seq $init_seed $tot_runs); do
            echo "$inst $($CMD -i $inst -s $s -v 1 -p $parser_type | grep "Best solution")" >>$inst_out
            printf "."
        done

        rm -f $base_inst_name.sol
        echo "100%"
    done
}

run_for_dataset "../instances/rail/rail*" RAIL $NRUNS rail
run_for_dataset "../instances/scp/*.txt" SCP $NRUNS scp
run_for_dataset "../instances/mps/*.mps" MPS $NRUNS mps
# run_for_dataset "../instances/cvrp/*.scp" CVRP $NRUNS cvrp
