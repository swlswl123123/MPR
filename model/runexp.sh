net=$1
duration=$2
seed=$3
# cp $net.tx.csv traffic.csv
find . -name "*.dll" | xargs rm -rf
find . -name "*.ot" | xargs rm -rf
find . -name "*.ov" | xargs rm -rf
# find . -name "*.csv" | xargs rm -rf
find . -name "*.lib" | xargs rm -rf
find . -name "*.exp" | xargs rm -rf
find . -name "*.obj" | xargs rm -rf
find . -name "*.pdb" | xargs rm -rf
sed -e 's/netname/'$net'/g' sim.ef > $net.ef
op_runsim -c -net_name $net -ef $net -duration $duration -seed $seed > ../sim/$net.log
# ./qgrep.sh "../sim/$net.log"
# ./nbgrep.sh "../sim/$net.log"
./dgrep.sh "../sim/$net.log"
# ./bgrep.sh "../sim/$net.log"
# ./lgrep.sh "../sim/$net.log"
# ./rgrep.sh "../sim/$net.log"
# ./ggrep.sh "../sim/$net.log"
# ./sgrep.sh "../sim/$net.log"
# ./urggrep.sh "../sim/$net.log"