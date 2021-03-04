//该文件放置在 $(KERNEL_SRC)/include/trace/events/
#undef TRACE_SYSTEM
#define TRACE_SYSTEM test

#if !defined(_TRACE_TEST_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_TEST_H

#include <linux/tracepoint.h>

TRACE_EVENT(test_tracepoint,

	TP_PROTO(int test_a, int test_b),

	TP_ARGS(test_a, test_b),

	TP_STRUCT__entry(
		__field(int ,	param1)
		__field(int ,	param2)
	),

	TP_fast_assign(
		__entry->param1    = test_a;
		__entry->param2    = test_b;
	),

	TP_printk("the test test_a(%d), test_b(%d)\n", 
              __entry->param1, __entry->param2)
);
#endif

#include <trace/define_trace.h>
