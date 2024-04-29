<!--
SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
SPDX-License-Identifier: MIT
-->

# Benchmarks
Benchmark have been run on ThinkPad P16v with an [`intel i7-13700H`](https://www.cpubenchmark.net/cpu.php?cpu=Intel+Core+i7-13700H) processor and 32GB of RAM on an Arch system, compiled with `g++ 13.2.1`.

We run the tests using the script located in [`benchmarks/run_all.sh`](../benchmarks/run_all.sh) with the project compiled in `Release` mode.

We tested each instance with 10 different random seeds.
The runs were not subject to any time limit, since we used the termination criterion described in the _Refinement_ section of the original paper.

## Rail Instances

| Instance                                  |   #Rows|    #Cols|  Best Sol |   Avg Sol | Avg Time(s) |
|:---                                       |    ---:|     ---:|       ---:|       ---:|         ---:|
| [`rail516`](../instances/rail/rail516)    |    516 |    47311|    182.00 |    182.00 |        2.10 |
| [`rail582`](../instances/rail/rail582)    |    582 |   55515 |    211.00 |    211.00 |        1.02 |
| [`rail507`](../instances/rail/rail507)    |    507 |   63009 |    174.00 |    175.00 |        3.77 |
| [`rail2586`](../instances/rail/rail2586)  |   2586 |  920683 |    950.00 |    950.80 |       57.14 |
| [`rail4872`](../instances/rail/rail4872)  |   4872 |  968672 |   1533.00 |   1534.60 |      160.09 |
| [`rail2536`](../instances/rail/rail2536)  |   2536 | 1081841 |    692.00 |    694.00 |       85.69 |
| [`rail4284`](../instances/rail/rail4284)  |   4284 | 1092610 |   1066.00 |   1067.50 |       91.58 |

_Note: For rail2586, better results can be obtained by emphasizing the quality of multipliers._

## SCP Instances

| Instance                                   |   #Rows|    #Cols|  Best Sol |   Avg Sol | Avg Time(s) |
|:---                                        |    ---:|     ---:|       ---:|       ---:|         ---:|
|[`scp410`](../instances/scp/scp410.txt)     |    200 |    1000 |    514.00 |    514.00 |        0.04 |
|[`scp41`](../instances/scp/scp41.txt)       |    200 |    1000 |    429.00 |    429.00 |        0.07 |
|[`scp42`](../instances/scp/scp42.txt)       |    200 |    1000 |    512.00 |    512.00 |        0.03 |
|[`scp43`](../instances/scp/scp43.txt)       |    200 |    1000 |    516.00 |    516.00 |        0.05 |
|[`scp44`](../instances/scp/scp44.txt)       |    200 |    1000 |    495.00 |    495.00 |        0.09 |
|[`scp45`](../instances/scp/scp45.txt)       |    200 |    1000 |    512.00 |    512.00 |        0.04 |
|[`scp46`](../instances/scp/scp46.txt)       |    200 |    1000 |    560.00 |    560.20 |        0.11 |
|[`scp47`](../instances/scp/scp47.txt)       |    200 |    1000 |    430.00 |    430.00 |        0.05 |
|[`scp48`](../instances/scp/scp48.txt)       |    200 |    1000 |    492.00 |    492.00 |        0.09 |
|[`scp49`](../instances/scp/scp49.txt)       |    200 |    1000 |    641.00 |    641.00 |        0.06 |
|[`scp510`](../instances/scp/scp510.txt)     |    200 |    2000 |    265.00 |    265.00 |        0.04 |
|[`scp51`](../instances/scp/scp51.txt)       |    200 |    2000 |    253.00 |    253.40 |        0.10 |
|[`scp52`](../instances/scp/scp52.txt)       |    200 |    2000 |    302.00 |    302.00 |        0.07 |
|[`scp53`](../instances/scp/scp53.txt)       |    200 |    2000 |    226.00 |    226.00 |        0.01 |
|[`scp54`](../instances/scp/scp54.txt)       |    200 |    2000 |    242.00 |    242.00 |        0.07 |
|[`scp55`](../instances/scp/scp55.txt)       |    200 |    2000 |    211.00 |    211.00 |        0.01 |
|[`scp56`](../instances/scp/scp56.txt)       |    200 |    2000 |    213.00 |    213.00 |        0.01 |
|[`scp57`](../instances/scp/scp57.txt)       |    200 |    2000 |    293.00 |    293.00 |        0.11 |
|[`scp58`](../instances/scp/scp58.txt)       |    200 |    2000 |    288.00 |    288.00 |        0.08 |
|[`scp59`](../instances/scp/scp59.txt)       |    200 |    2000 |    279.00 |    279.00 |        0.05 |
|[`scp61`](../instances/scp/scp61.txt)       |    200 |    1000 |    138.00 |    138.40 |        0.34 |
|[`scp62`](../instances/scp/scp62.txt)       |    200 |    1000 |    146.00 |    146.00 |        0.29 |
|[`scp63`](../instances/scp/scp63.txt)       |    200 |    1000 |    145.00 |    145.00 |        0.21 |
|[`scp64`](../instances/scp/scp64.txt)       |    200 |    1000 |    131.00 |    131.00 |        0.27 |
|[`scp65`](../instances/scp/scp65.txt)       |    200 |    1000 |    161.00 |    161.00 |        0.20 |
|[`scpa1`](../instances/scp/scpa1.txt)       |    300 |    3000 |    253.00 |    253.70 |        0.54 |
|[`scpa2`](../instances/scp/scpa2.txt)       |    300 |    3000 |    252.00 |    252.10 |        0.54 |
|[`scpa3`](../instances/scp/scpa3.txt)       |    300 |    3000 |    232.00 |    233.40 |        0.49 |
|[`scpa4`](../instances/scp/scpa4.txt)       |    300 |    3000 |    234.00 |    234.00 |        0.26 |
|[`scpa5`](../instances/scp/scpa5.txt)       |    300 |    3000 |    236.00 |    236.40 |        0.29 |
|[`scpb1`](../instances/scp/scpb1.txt)       |    300 |    3000 |     69.00 |     69.00 |        0.54 |
|[`scpb2`](../instances/scp/scpb2.txt)       |    300 |    3000 |     76.00 |     76.00 |        0.52 |
|[`scpb3`](../instances/scp/scpb3.txt)       |    300 |    3000 |     80.00 |     80.00 |        0.57 |
|[`scpb4`](../instances/scp/scpb4.txt)       |    300 |    3000 |     79.00 |     79.00 |        0.48 |
|[`scpb5`](../instances/scp/scpb5.txt)       |    300 |    3000 |     72.00 |     72.00 |        0.47 |
|[`scpc1`](../instances/scp/scpc1.txt)       |    400 |    4000 |    227.00 |    227.20 |        0.74 |
|[`scpc2`](../instances/scp/scpc2.txt)       |    400 |    4000 |    219.00 |    219.00 |        0.59 |
|[`scpc3`](../instances/scp/scpc3.txt)       |    400 |    4000 |    243.00 |    243.00 |        0.88 |
|[`scpc4`](../instances/scp/scpc4.txt)       |    400 |    4000 |    219.00 |    219.10 |        0.72 |
|[`scpc5`](../instances/scp/scpc5.txt)       |    400 |    4000 |    215.00 |    215.50 |        0.61 |
|[`scpclr10`](../instances/scp/scpclr10.txt) |    511 |     210 |     25.00 |     25.00 |        0.84 |
|[`scpclr11`](../instances/scp/scpclr11.txt) |   1023 |     330 |     23.00 |     23.00 |        2.65 |
|[`scpclr12`](../instances/scp/scpclr12.txt) |   2047 |     495 |     23.00 |     23.00 |        9.56 |
|[`scpclr13`](../instances/scp/scpclr13.txt) |   4095 |     715 |     26.00 |     26.00 |        5.39 |
|[`scpcyc06`](../instances/scp/scpcyc06.txt) |    240 |     192 |     60.00 |     60.50 |        0.28 |
|[`scpcyc07`](../instances/scp/scpcyc07.txt) |    672 |     448 |    144.00 |    146.50 |        2.08 |
|[`scpcyc08`](../instances/scp/scpcyc08.txt) |   1792 |    1024 |    348.00 |    351.00 |       14.24 |
|[`scpcyc09`](../instances/scp/scpcyc09.txt) |   4608 |    2304 |    792.00 |    812.30 |       86.36 |
|[`scpcyc10`](../instances/scp/scpcyc10.txt) |  11520 |    5120 |   1816.00 |   1853.40 |      471.23 |
|[`scpcyc11`](../instances/scp/scpcyc11.txt) |  28160 |   11264 |   4069.00 |   4155.90 |     3150.97 |
|[`scpd1`](../instances/scp/scpd1.txt)       |    400 |    4000 |     60.00 |     60.00 |        0.64 |
|[`scpd2`](../instances/scp/scpd2.txt)       |    400 |    4000 |     66.00 |     66.00 |        0.76 |
|[`scpd3`](../instances/scp/scpd3.txt)       |    400 |    4000 |     72.00 |     72.00 |        0.65 |
|[`scpd4`](../instances/scp/scpd4.txt)       |    400 |    4000 |     62.00 |     62.00 |        0.66 |
|[`scpd5`](../instances/scp/scpd5.txt)       |    400 |    4000 |     61.00 |     61.00 |        0.69 |
|[`scpe1`](../instances/scp/scpe1.txt)       |     50 |     500 |      5.00 |      5.00 |        0.01 |
|[`scpe2`](../instances/scp/scpe2.txt)       |     50 |     500 |      5.00 |      5.00 |        0.01 |
|[`scpe3`](../instances/scp/scpe3.txt)       |     50 |     500 |      5.00 |      5.00 |        0.02 |
|[`scpe4`](../instances/scp/scpe4.txt)       |     50 |     500 |      5.00 |      5.00 |        0.02 |
|[`scpe5`](../instances/scp/scpe5.txt)       |     50 |     500 |      5.00 |      5.00 |        0.02 |
|[`scpnre1`](../instances/scp/scpnre1.txt)   |    500 |    5000 |     29.00 |     29.00 |        1.09 |
|[`scpnre2`](../instances/scp/scpnre2.txt)   |    500 |    5000 |     30.00 |     30.80 |        1.20 |
|[`scpnre3`](../instances/scp/scpnre3.txt)   |    500 |    5000 |     27.00 |     27.00 |        0.95 |
|[`scpnre4`](../instances/scp/scpnre4.txt)   |    500 |    5000 |     28.00 |     28.00 |        1.21 |
|[`scpnre5`](../instances/scp/scpnre5.txt)   |    500 |    5000 |     28.00 |     28.00 |        1.07 |
|[`scpnrf1`](../instances/scp/scpnrf1.txt)   |    500 |    5000 |     14.00 |     14.00 |        0.84 |
|[`scpnrf2`](../instances/scp/scpnrf2.txt)   |    500 |    5000 |     15.00 |     15.00 |        0.79 |
|[`scpnrf3`](../instances/scp/scpnrf3.txt)   |    500 |    5000 |     14.00 |     14.00 |        0.83 |
|[`scpnrf4`](../instances/scp/scpnrf4.txt)   |    500 |    5000 |     14.00 |     14.00 |        0.79 |
|[`scpnrf5`](../instances/scp/scpnrf5.txt)   |    500 |    5000 |     13.00 |     13.30 |        0.99 |
|[`scpnrg1`](../instances/scp/scpnrg1.txt)   |   1000 |   10000 |    176.00 |    176.10 |        3.19 |
|[`scpnrg2`](../instances/scp/scpnrg2.txt)   |   1000 |   10000 |    154.00 |    154.80 |        3.84 |
|[`scpnrg3`](../instances/scp/scpnrg3.txt)   |   1000 |   10000 |    167.00 |    167.00 |        4.35 |
|[`scpnrg4`](../instances/scp/scpnrg4.txt)   |   1000 |   10000 |    168.00 |    169.20 |        3.99 |
|[`scpnrg5`](../instances/scp/scpnrg5.txt)   |   1000 |   10000 |    168.00 |    168.20 |        4.00 |
|[`scpnrh1`](../instances/scp/scpnrh1.txt)   |   1000 |   10000 |     64.00 |     64.00 |        3.15 |
|[`scpnrh2`](../instances/scp/scpnrh2.txt)   |   1000 |   10000 |     63.00 |     63.40 |        4.10 |
|[`scpnrh3`](../instances/scp/scpnrh3.txt)   |   1000 |   10000 |     60.00 |     60.00 |        4.36 |
|[`scpnrh4`](../instances/scp/scpnrh4.txt)   |   1000 |   10000 |     58.00 |     58.80 |        3.58 |
|[`scpnrh5`](../instances/scp/scpnrh5.txt)   |   1000 |   10000 |     55.00 |     55.00 |        3.17 |

## MPS Instances
| Instance      |   #Rows|    #Cols|  Best Sol |   Avg Sol | Avg Time(s) |
|:---           |    ---:|     ---:|       ---:|       ---:|         ---:|
|`ramos3`       |   2187 |    2187 |    194.00 |    206.50 |       14.39 |
|`ex1010-pi`    |   1468 |   25200 |    236.00 |    237.60 |        5.34 |
|`scpk4`        |   2000 |  100000 |    323.00 |    324.90 |       17.78 |
|`scpj4scip`    |   1000 |   99947 |    129.00 |    131.80 |        9.59 |
|`scpl4`        |   2000 |  200000 |    262.00 |    265.80 |       25.69 |
|`scpm1`        |   5000 |  500000 |    546.00 |    551.00 |      138.98 |
|`scpn2`        |   5000 | 1000000 |    500.00 |    502.00 |      166.05 |
