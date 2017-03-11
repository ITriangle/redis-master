#!/bin/sh

# host="192.168.8.62"
# port="6379"
# password="ele-fence@2015#seentech"

host=$1
port=$2
password=$3
data=$4
cat $data | ./redis-cli -h $host -p $port -a $password --pipe
