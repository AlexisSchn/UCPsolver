#!/bin/sh
# chmod +x the_file_name


# keys
n=20
T=24
bloc=1 #instance type
demand=3 #demand type. 3 --> 2-peaks per day demand; 4 = random
sym=0    #level of symmetry. 0--> no symmetry ; n--> all units are identical ; otherwise, generates n/sym units identical in average
binary=1  # 1 --> Pmin=Pmax
intra=0 #generates sites
dossier=../data/${n}-${T}-eq/

solver=soplex

# We initialize the exit_data.csv
cd ..
cd exit
> exit_data.csv
echo "Decomposition & coeff fixed & coeff prop & iter & cols u & cols t & CPU & CPUmp & CPUpp & LB & gap & \\\\\\ \hline  % ${n}_${T}_${bloc}_${demand}_${sym}_${binary}_${intra}" >> exit_data.csv
cd ..

# We go over the instances and execute the solving
# Compilation

if [ $solver = cplex ]
then
    echo "Solving with CPLEX ... "
    make LPS=cpx ZIMPL=false
fi

if [ $solver = soplex ]
then
    echo "Solving with Soplex ... "
    make
fi

# Solving
cd bin
for i in $(seq 1 1 10);
do
    echo "Starting solving of instance $i ..."
    data_path=${dossier}${n}_${T}_${bloc}_${demand}_${sym}_${binary}_${intra}_${i}.txt
    echo $data_path
    ./ucp.exe $data_path $data ../exit/exit_data.csv 1000 0
done

