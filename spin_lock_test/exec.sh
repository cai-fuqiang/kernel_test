#!/bin/sh
_exec()
{
        exec_name=$1
        echo exec_name: $exec_name  num: $NEED_EXEC_NUM
        for ((i=0;i<$NEED_EXEC_NUM;i++))
        do
                if [ $i -eq 0 ] ;then
                        perf stat -e machine_clears.memory_ordering -e inst_retired.any ./$exec_name > log/${exec_name}.txt 2>&1 &
                else
                        perf stat -e machine_clears.memory_ordering -e inst_retired.any ./$exec_name > /dev/null 2>&1 &
                fi
        done
}

_wait()
{
        while [ 1 ]
        do
        	CUR_EXEC_NUM=`ps aux |grep spinlock |wc -l`
        
        	if [ $CUR_EXEC_NUM -eq 1 ];then
        		break
        	fi
        done
        echo wait done
}

exec_and_wait()
{
        _exec $1
        _wait $1
}

export NEED_EXEC_NUM=40 XCHG_BEG=1
make clean
make

#exec_and_wait spinlock_pause
#exec_and_wait spinlock_nopause
