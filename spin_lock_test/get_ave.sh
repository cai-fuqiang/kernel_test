LOG_FILE_DIR="log"
count_ave()
{
	LOG_FILE_PREV=$1
	GROP_EVENTS=$2
	IS_FLOAT=$3
	total=0
	i=0;
	for DATA in `cat ${LOG_FILE_DIR}/${LOG_FILE_PREV}* |grep "$GROP_EVENTS" |awk '{print $1}' |sed 's/,//g'`
	do
		total=`echo "$total $DATA + p"|dc`
		#echo one data is $DATA
		((i=i+1))
	done
	#echo i = $i
	#echo total = $total
	#echo event = $GROP_EVENTS
	if [ $IS_FLOAT == 0 ];then
		ave=`echo "$total $i / p" | dc`
		ave_t=`echo $ave |awk '{printf "%\047d\n", $0}' `
		printf "%20s\t" "${ave_t}" 
		printf "%s\n" "${GROP_EVENTS}"
	else
		ave=`echo "9k $total $i / p" | dc`
		printf "%20f\t" "${ave}" 
		printf "%s %s %s\n" "${GROP_EVENTS}"
	fi
	#printf "%20f\t%s%s%s" "${ave}" ${GROP_EVENTS}
	
}

count_ave_and_print()
{
	LOG_FILE_PREV=$1
	GROP_EVENTS=$2
	IS_FLOAT=$3
	ave=$(count_ave $LOG_FILE_PREV $GROP_EVENTS $IS_FLOAT)
	echo -E ${LOG_FILE_PREV}"\t"$ave
}

get_one_version()
{
	LOG_FILE_PREV=$1
	echo  "Performance counter stats for '${LOG_FILE_PREV}':"
	count_ave $LOG_FILE_PREV 'machine_clears.memory_ordering:u' 0
	count_ave $LOG_FILE_PREV 'inst_retired.any:u' 0
	count_ave $LOG_FILE_PREV 'cycles:u' 0
        count_ave $LOG_FILE_PREV 'task-clock:u' 0
	count_ave $LOG_FILE_PREV 'seconds time elapsed' 1
	count_ave $LOG_FILE_PREV "seconds user" 1
	count_ave $LOG_FILE_PREV "seconds sys" 1
}

get_log_dir()
{
	if [ -n "$1" ];then
		LOG_FILE_DIR=$1
	fi
}

get_log_dir $1
get_one_version spinlock_pause
get_one_version spinlock_nopause
#get_one_version only_loop_spinlock_pause
#get_one_version only_loop_spinlock_nopause
