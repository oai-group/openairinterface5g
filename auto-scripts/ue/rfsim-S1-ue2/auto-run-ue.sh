#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function isValidIp() {
    local ip=$1
    local ret=1

    if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
        ip=(${ip//\./ })
        [[ ${ip[0]} -le 255 && ${ip[1]} -le 255 && ${ip[2]} -le 255 && ${ip[3]} -le 255 ]]
        ret=$?
    fi

    return $ret
}

function SET_ENB() {
	while true; do
	    read -p "请输入当前UE的eNB接口IP地址: " INPUT_eNB
	    if isValidIp $INPUT_eNB
	        then
				eNB_Addr=$INPUT_eNB
				return 0
            else
	            echo "您输入的IP无效"    
            fi
	done
}

while getopts ":a:" opt
do
    case $opt in
        a)if isValidIp $OPTARG
	        then
                eNB_Addr=$OPTARG
            else
	            echo "您输入的IP无效"    
            fi
        ;;
        ?)echo "您输入的参数无效"
        ;;
    esac
done

if [ ! -n "$eNB_Addr" ]; then  
    SET_ENB  
else  
    echo "已通过-a参数配置当前UE的eNB接口IP地址: $eNB_Addr"  
fi

cd ${DIR}/../../..
source oaienv
cd cmake_targets/ran_build/build
../../nas_sim_tools/build/conf2uedata -c ${DIR}/sim.conf -o .
sudo RFSIMULATOR=$eNB_Addr ./lte-uesoftmodem -C 2685000000 -r 25 --rfsim