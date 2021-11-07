#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASECONFIG="$DIR/enb.10MHz.b200.conf"
CONFIGFILE="$DIR/enb.conf"

function SET_CI_ENB() {
    local NUM ARRAY_LENGTH 
    NUM=0
    #for NIC_NAME in $(ls /sys/class/net|grep -vE "lo|docker0"); do
    for NIC_NAME in $(ls /sys/class/net); do
        NIC_IP=$(ifconfig $NIC_NAME |awk -F'[: ]+' '/inet /{print $3}')
        if [ -n "$NIC_IP" ]; then
            NIC_IP_ARRAY[$NUM]="$NIC_NAME:$NIC_IP"
            let NUM++
        fi
    done
    ARRAY_LENGTH=${#NIC_IP_ARRAY[*]}
    if [ $ARRAY_LENGTH -eq 1 ]; then     
		echo "当前设备仅有一张网卡,默认选择此网卡"
        CI_ENB_INTERFACE=${NIC_IP_ARRAY[$INPUT_NIC_NUM]%:*}
		CI_ENB_IP_ADDR=${NIC_IP_ARRAY[$INPUT_NIC_NUM]#*:}
        return 0
    elif [ $ARRAY_LENGTH -eq 0 ]; then
        echo "当前设备没有合适的网卡"
        exit 1
    else
        while true; do
        	ID=0
            for NIC in ${NIC_IP_ARRAY[*]}; do
                let ID+=1
                echo "("$ID"):"$NIC
            done
            read -p "请为当前eNB选择本地MME、S1U、X2C接口网卡[1-$ID]: " INPUT_NIC_NUM
            test=$(echo $INPUT_NIC_NUM | sed 's/[0-9]//g')
            if [ -n "$INPUT_NIC_NUM" -a -z "$test" ]; then
                if [ $ID -ge $INPUT_NIC_NUM -a $INPUT_NIC_NUM -ge 1 ]; then
                    let INPUT_NIC_NUM-=1
                    CI_ENB_INTERFACE=${NIC_IP_ARRAY[$INPUT_NIC_NUM]%:*}
                    CI_ENB_IP_ADDR=${NIC_IP_ARRAY[$INPUT_NIC_NUM]#*:}
					echo "----------------------"
                    return 0
                else
                    echo "请在给定的选项中进行选择"
                fi
            else
                echo "请输入正确的选项[1-$ID]" 
            fi
        done
    fi
}

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

function SET_CI_MME() {
	while true; do
	    read -p "请为当前eNB选择EPC的MME接口IP地址: " INPUT_EPC_MME
	    if isValidIp $INPUT_EPC_MME
	        then
				CI_MME_IP_ADDR=$INPUT_EPC_MME
				return 0
            else
	            echo "您输入的IP无效"    
            fi
	done
}


while getopts ":a:r" opt
do
    case $opt in
        a)
		if isValidIp $OPTARG; then
			CI_MME_IP_ADDR=$OPTARG
		else
			echo "Invalid IP"    
        fi
        ;;
        r)
		if [ -f "$CONFIGFILE" ]; then
			rm $CONFIGFILE
			echo "重新配置eNB配置文件"
		fi
        ;;
        ?)echo "Invalid args"
        ;;
    esac
done


if [ ! -f "$CONFIGFILE" ]; then
	if [ ! -f "$BASECONFIG" ]; then
		echo "enb.10MHz.b200.conf文件不存在,请先获取enb.10MHz.b200.conf文件"
		exit 1
	fi
	
	cp $BASECONFIG $CONFIGFILE
	SET_CI_ENB
	if [ ! -n "$CI_MME_IP_ADDR" ]; then  
		SET_CI_MME  
	else  
		echo "已通过-a参数配置当前EPC的MME接口IP地址: $CI_MME_IP_ADDR"  
	fi
	sed -i "s/CI_ENB_INTERFACE/$CI_ENB_INTERFACE/g" $CONFIGFILE
	sed -i "s/CI_ENB_IP_ADDR/$CI_ENB_IP_ADDR/g" $CONFIGFILE
	sed -i "s/CI_MME_IP_ADDR/$CI_MME_IP_ADDR/g" $CONFIGFILE
	echo "配置文件enb.conf生成成功,配置如下:"
	echo "mme_ip_address= $CI_MME_IP_ADDR;"
    echo "ENB_INTERFACE_NAME_FOR_S1_MME = $CI_ENB_INTERFACE;"	
    echo "ENB_IPV4_ADDRESS_FOR_S1_MME = $CI_ENB_IP_ADDR;"	
    echo "ENB_INTERFACE_NAME_FOR_S1U = $CI_ENB_INTERFACE;"	
    echo "ENB_IPV4_ADDRESS_FOR_S1U = $CI_ENB_IP_ADDR;"	
    echo "ENB_IPV4_ADDRESS_FOR_X2C = $CI_ENB_IP_ADDR;"
else
	echo "检测到当前存在配置文件enb.conf,使用此文件启动eNB"
fi

cd ${DIR}/../../..
source oaienv
cd cmake_targets/ran_build/build
sudo RFSIMULATOR=enb ./lte-softmodem -O ${DIR}/enb.conf --rfsim