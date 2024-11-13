#!/bin/bash


(
sleep 3
echo "1"   
sleep 1
echo "2"
sleep 1
echo "2"   
sleep 1
echo "0 1 10" 
sleep 1
echo "4"  
sleep 1
echo "1"
sleep 1
echo "5"  
sleep 1
echo "0"  
sleep 1
echo "4"
sleep 1
echo "2"
sleep 1
echo "0"  
sleep 1
echo "6"  
sleep 1
echo "7"  
sleep 1
echo "8"  
sleep 1
echo "9"  
sleep 1
echo "0"  
sleep 1
echo "1"  
sleep 1
echo "2"  
sleep 1
echo "0"  
sleep 1
echo "9"  
sleep 1
echo "10" 
) | telnet localhost 4040


