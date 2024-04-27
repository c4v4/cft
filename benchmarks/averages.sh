#!/bin/bash
# SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
# SPDX-License-Identifier: MIT

compute_averages() {
    pattern=$1
    out_dir=$2
    for inst in $(ls -rS $pattern); do
        base_inst_name=$(basename -- $inst)
        base_inst_name=${base_inst_name%.*}
        inst_out=$out_dir/$base_inst_name.out
        if [ -f "$inst_out" ]; then
            min_sol=$(awk 'NR == 1 || $5 < min {min = $5}END {print min}' $inst_out)
            avg_sol=$(awk '{sum += $5} END {print sum / NR}' $inst_out)
            avg_time=$(awk '{sum += $7} END {print sum / NR}' $inst_out)
            printf "%-16s %10.2f %10.2f %10.2f\n" $base_inst_name $min_sol $avg_sol $avg_time
        fi
    done
}

compute_averages "../instances/rail/rail*" rail
echo
compute_averages "../instances/scp/*.txt" scp
echo
compute_averages "../instances/mps/*.mps" mps

#compute_averages "../instances/cvrp/*.scp" cvrp
