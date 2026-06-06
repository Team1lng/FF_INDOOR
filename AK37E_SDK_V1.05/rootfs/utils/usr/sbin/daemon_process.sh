#!/bin/sh
# 守护进程
# while test "1" = "1"
# do
#     sleep 1
#     value=$(ps aux | grep FF.BIN | grep -v grep | wc -l)
#     if [ $value -ne 1 ]
#     then
#         killall FF.BIN
#         /app/app/FF.BIN leo &
#     fi
# done



while test "1" = "1"
do
    sleep 1
    value=$(ps aux | grep FF.BIN | grep -v grep | wc -l)
    if [ $value -eq 0 ]
    then
        killall FF.BIN
        /app/app/FF.BIN leo &
        sleep 2
    fi
done


