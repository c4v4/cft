#!/bin/bash

dir=/home/rop/repos/scp
exe=${dir}/build/accft

declare -a cvrp_instances=(
    "X-n120-k6.vrp_seed-0_z14949.000000.scp" "X-n469-k138_z223666_cplex223591.scp" "X-n536-k96_z95480_cplex95479.scp"
    "X-n573-k30_z51112_cplex51109.scp" "X-n819-k171_z159168_cplex159561err.scp" "X-n819-k171_z159584_cplex159577.scp"
    "X-n819-k171.vrp_seed-0_z159081.000000.scp" "X-n1001-k43_z73001_cplex72987.scp"
)

declare -a rail_instances=(
    "rail507" "rail516" "rail582" "rail2536" "rail2586" "rail4284" "rail4872"
)

declare -a scp_instances=(
    "scp41.txt" "scp42.txt" "scp43.txt""scp44.txt" "scp45.txt" "scp46.txt" "scp47.txt" "scp48.txt" "scp49.txt" "scp51.txt" "scp52.txt" "scp53.txt" "scp54.txt" "scp55.txt"
    "scp56.txt" "scp57.txt" "scp58.txt" "scp59.txt" "scp61.txt" "scp62.txt" "scp63.txt" "scp64.txt" "scp65.txt" "scp410.txt" "scp510.txt" "scpa1.txt" "scpa2.txt" "scpa3.txt"
    "scpa4.txt" "scpa5.txt" "scpb1.txt" "scpb2.txt" "scpb3.txt" "scpb4.txt" "scpb5.txt" "scpc1.txt" "scpc2.txt" "scpc3.txt" "scpc4.txt" "scpc5.txt" "scpclr10.txt"
    "scpclr11.txt" "scpclr12.txt" "scpclr13.txt" "scpcyc06.txt" "scpcyc07.txt" "scpcyc08.txt" "scpcyc09.txt" "scpcyc10.txt" "scpcyc11.txt" "scpd1.txt" "scpd2.txt"
    "scpd3.txt" "scpd4.txt" "scpd5.txt" "scpe1.txt" "scpe2.txt" "scpe3.txt" "scpe4.txt" "scpe5.txt" "scpnre1.txt" "scpnre2.txt" "scpnre3.txt" "scpnre4.txt" "scpnre5.txt"
    "scpnrf1.txt" "scpnrf2.txt" "scpnrf3.txt" "scpnrf4.txt" "scpnrf5.txt" "scpnrg1.txt" "scpnrg2.txt" "scpnrg3.txt" "scpnrg4.txt" "scpnrg5.txt" "scpnrh1.txt"
    "scpnrh2.txt" "scpnrh3.txt" "scpnrh4.txt" "scpnrh5.txt"
)

for seed in {0..9..1}; do
    for inst in "${cvrp_instances[@]}"; do
        ${exe} ${dir}/instances/cvrp/${inst} --seed ${seed} --parser CVRP
    done
    for inst in "${rail_instances[@]}"; do
        ${exe} ${dir}/instances/rail/${inst} --seed ${seed} --parser RAILS
    done
    for inst in "${scp_instances[@]}"; do
        ${exe} ${dir}/instances/scp/${inst} --seed ${seed} --parser SCP
    done
done
