#!/bin/sh
TEST_ONLY_LOOP=0
_exec()
{
        exec_name=$1
	log_suffix=$2
        echo exec_name: $exec_name  num: $NEED_EXEC_NUM
        for ((i=0;i<$NEED_EXEC_NUM;i++))
        do
                perf stat -e machine_clears.memory_ordering -e inst_retired.any -e cycles -e task-clock ./$exec_name > log/${exec_name}_${log_suffix}_${i}.txt 2>&1 &
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
        _exec $1 ""
	if [ $TEST_ONLY_LOOP  -eq 1 ];then
		_exec only_loop $1
	fi
        _wait $1
	if [ $TEST_ONLY_LOOP  -eq 1 ];then
		killall only_loop
	fi
}
mkdir -p log
export NEED_EXEC_NUM=40 XCHG_BEG=0
make clean
make

exec_and_wait spinlock_pause
exec_and_wait spinlock_nopause
