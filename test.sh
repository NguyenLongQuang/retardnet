sudo rmmod retardnet
sudo insmod retardnet.ko
sudo ./rtnet $1
iperf3 -s
