echo "$$" > /tmp/test_pid

QcaWanMacAddr="$(cat /raymarine/Conf/FixedData_QcaWanMacAddr)"

trap "echo -n "$(ip -4 neighbour | grep eth0 | grep -i $QcaWanMacAddr | awk '{print $1}')" > /tmp/qca9563_ip_addr" SIGRTMIN+1

while true;do
        sleep 1
done
