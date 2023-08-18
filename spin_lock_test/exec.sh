#!/bin/sh
make clean
export NEED_EXEC_NUM=40
make

mkdir -p log
if [ "$1" != "nopause" ];then
	exec_name=./spinlock_pause 
else
	exec_name=./spinlock_nopause 
fi

for ((i=0;i<$NEED_EXEC_NUM;i++))
do
perf stat -e machine_clears.memory_ordering -e inst_retired.any ./$exec_name > log/${exec_name}_${i}.txt 2>&1 &
done

while [ 1 ]
do
	CUR_EXEC_NUM=`ps aux |grep spinlock |wc -l`

	if [ $CUR_EXEC_NUM -eq 1 ];then
		break
	fi
done
