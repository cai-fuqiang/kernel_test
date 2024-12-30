#include <stdio.h>
#include <stdint.h>

int ring_full_time_us = 200000;

void org(uint64_t quota, uint64_t current) 
{
	uint64_t sleep_pct = 0;
	uint64_t throttle_us = 0;

	sleep_pct = (current - quota) * 100 / current;
	throttle_us =
		ring_full_time_us * sleep_pct / (double)(100 - sleep_pct);

	printf("%s:\t\tcurrent(%llu) quota(%llu) sleep_pct(%llu) throttle_us(%llu)\n", 
			__func__,
			current, quota, sleep_pct, throttle_us);
	printf("after throttle dirtyrate %llu\n", (current * ring_full_time_us) / (throttle_us + ring_full_time_us));
	return;
}
void org_chg_100_to_10000(uint64_t quota, uint64_t current)
{
	uint64_t sleep_pct = 0;
	uint64_t throttle_us = 0;

	sleep_pct = (current - quota) * 10000 / current;
	throttle_us =
		ring_full_time_us * sleep_pct / (double)(10000 - sleep_pct);

	printf("%s:\t\tcurrent(%llu) quota(%llu) sleep_pct(%llu) throttle_us(%llu)\n", 
			__func__,
			current, quota, throttle_us * 100/(throttle_us + ring_full_time_us), throttle_us);
	printf("after throttle dirtyrate %llu\n", (current * ring_full_time_us) / (throttle_us + ring_full_time_us));
	return;
}

void my_func_use_double(uint64_t quota, uint64_t current)
{
	double sleep_pct = 0;
	uint64_t throttle_us = 0;

	sleep_pct = (current - quota) / quota;
	throttle_us =
		ring_full_time_us * sleep_pct;

	printf("%s:\t\tcurrent(%llu) quota(%llu) sleep_pct(%llu) throttle_us(%llu)\n", 
			__func__,
			current, quota, throttle_us * 100/(throttle_us + ring_full_time_us), throttle_us);
	printf("after throttle dirtyrate %llu\n", (current * ring_full_time_us) / (throttle_us + ring_full_time_us));
	return;
}

void my_func_use_int_100(uint64_t quota, uint64_t current)
{
	uint64_t sleep_pct = 0;
	uint64_t throttle_us = 0;

	sleep_pct = (current - quota) * 100/ quota;
	throttle_us =
		ring_full_time_us * sleep_pct / 100;

	printf("%s:\t\tcurrent(%llu) quota(%llu) sleep_pct(%llu) throttle_us(%llu)\n", 
			__func__,
			current, quota, throttle_us * 100/(throttle_us + ring_full_time_us), throttle_us);
	printf("after throttle dirtyrate %llu\n", (current * ring_full_time_us) / (throttle_us + ring_full_time_us));
	return;
}


void test_batch(uint64_t quota, uint64_t current)
{
	printf("=========================\n");
	printf("curent(%llu) quota(%llu) \n", current, quota);
	printf("=========================\n");
	org   (quota, current);
	printf("-------------------------\n");
	org_chg_100_to_10000(quota, current);
	printf("-------------------------\n");
	my_func_use_double(quota, current);
	printf("-------------------------\n");
	my_func_use_int_100(quota, current);
	printf("=========================\n\n");

}
int main()
{ 
	test_batch(333,1000);
	test_batch(333,1000000);
	test_batch(333,10000000000);
}
