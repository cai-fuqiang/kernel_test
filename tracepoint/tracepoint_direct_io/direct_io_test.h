//该文件放置在 $(KERNEL_SRC)/include/trace/events/
#undef TRACE_SYSTEM
#define TRACE_SYSTEM direct_io_test

#if !defined(_TRACE_DIRECT_IO_TEST_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_DIRECT_IO_TEST_H

#include <linux/uio.h>
#include <linux/tracepoint.h>
#include <linux/types.h>
#include <linux/page_ref.h>
#include <linux/tracepoint.h>
#include <trace/events/mmflags.h>
#include <linux/blk_types.h>
DECLARE_EVENT_CLASS(deb_iov_iter,

	TP_PROTO(struct iov_iter *iter),

	TP_ARGS(iter),

	TP_STRUCT__entry(
		__field(int, type)
		__field(size_t, iov_offset)
		__field(size_t, count)
		__field(void *, iov_base)
		__field(size_t, iov_len)
	),

	TP_fast_assign(
		__entry->type= iter->type;
		__entry->iov_offset= iter->iov_offset;
		__entry->count = iter->count;
		__entry->iov_base = iter->kvec->iov_base;
		__entry->iov_len= iter->kvec->iov_len;
	),

	TP_printk("iov_iter: type(%d) iov_offset(%lu) count(%lu) kvec.iov_base(%p) kvec.iov_len(%lu)", 
		__entry->type,
		__entry->iov_offset,
		__entry->count,
		__entry->iov_base,
		__entry->iov_len)

);

DEFINE_EVENT(deb_iov_iter, loop_beg_deb_iov_iter,

	TP_PROTO(struct iov_iter *iter),

	TP_ARGS(iter)

);

DEFINE_EVENT(deb_iov_iter, gup_beg_deb_iov_iter,

	TP_PROTO(struct iov_iter *iter),

	TP_ARGS(iter)
);

DEFINE_EVENT(deb_iov_iter, gup_end_deb_iov_iter,

	TP_PROTO(struct iov_iter *iter),

	TP_ARGS(iter)
);

DECLARE_EVENT_CLASS(deb_page,

    TP_PROTO(struct page *page),

    TP_ARGS(page),

    TP_STRUCT__entry(
        __field(unsigned long, pfn)
        __field(unsigned long, flags)
        __field(int, count)
        __field(int, mapcount)
        __field(void *, mapping)
        __field(int, mt)
    ),

    TP_fast_assign(
        __entry->pfn = page_to_pfn(page);
        __entry->flags = page->flags;
        __entry->count = page_ref_count(page);
        __entry->mapcount = page_mapcount(page);
        __entry->mapping = page->mapping;
        __entry->mt = get_pageblock_migratetype(page);
    ),

    TP_printk("pfn=0x%lx flags=%s count=%d mapcount=%d mapping=%p mt=%d",
        __entry->pfn,
        show_page_flags(__entry->flags & ((1UL << NR_PAGEFLAGS) - 1)),
        __entry->count,
        __entry->mapcount, __entry->mapping, __entry->mt)
);

DEFINE_EVENT(deb_page, gup_end_deb_page,

    TP_PROTO(struct page *page),

    TP_ARGS(page)
);

DECLARE_EVENT_CLASS(deb_func_name,

    TP_PROTO(char *msg),

	TP_ARGS(msg),

	TP_STRUCT__entry(
		__field(char *, msg)
	),
	
	TP_fast_assign(
		__entry->msg = msg;
	),

	TP_printk("msg is %s",
		__entry->msg)
);

DEFINE_EVENT(deb_func_name, iov_iter_get_pages_beg,

    TP_PROTO(char *msg),

	TP_ARGS(msg)

);

DEFINE_EVENT(deb_func_name,  iov_iter_get_pages_end,

    TP_PROTO(char *msg),

	TP_ARGS(msg)

);

DEFINE_EVENT(deb_func_name, iov_iter_get_pages_loop_end,

    TP_PROTO(char *msg),

	TP_ARGS(msg)
);

DEFINE_EVENT(deb_func_name,  iov_iter_get_pages_ret_res_le_0,

    TP_PROTO(char *msg),

	TP_ARGS(msg)
);

DEFINE_EVENT(bio_iov_deb, bio_iov_deb_beg,

	TP_PROTO(struct iov_iter *iter),

    TP_ARGS(iter),

);

DEFINE_EVENT(bio_iov_deb, bio_iov_deb_end,

	TP_PROTO(struct iov_iter *iter),

    TP_ARGS(iter),

);

DECLARE_EVENT_CLASS(bio_msg_deb,

    TP_PROTO(char *msg),

	TP_ARGS(msg),

	TP_STRUCT__entry(
		__field(char *, msg)
	),
	
	TP_fast_assign(
		__entry->msg = msg;
	),

	TP_printk("msg is %s",
		__entry->msg)
);

DEFINE_EVENT(bio_msg_deb,  bio_msg_deb_loop,

    TP_PROTO(char *msg),

	TP_ARGS(msg),
);

DECLARE_EVENT_CLASS(bio_deb,

	TP_PROTO(struct bio *bio),

	TP_ARGS(bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
    ),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
    ),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

DEFINE_EVENT(bio_deb, bio_deb_once,

	TP_PROTO(struct bio *bio),

	TP_ARGS(bio),

);

DEFINE_EVENT_CLASS(bio_io_vec_deb,

    TP_PROTO(struct bio *bio, int idx),

    TP_ARGS(bio, idx),

    TP_STRUCT__entry(
        __filed(unsigned int, idx)
        __field(unsigned long ,pfn)
        __filed(unsigned int , bv_len)
        __filed(unsigned int , bv_offset)
    ),

	TP_fast_assign(
        __entry->idx        = page_to_pfn(idx);
        __entry->pfn        = page_to_pfn(bio->bi_io_vec[idx].bv_page);
        __entry->bv_len     = bio->bi_io_vec[idx].bv_len;
        __entry->bv_offset  = bio->bi_io_vec[idx].bv_offset;
    ),

    TP_printk("bi_io_vec idx(%d) pfn(%lu) len(%u) offset(%u)",
              __entry->idx,
              __entry->pfn,
              __entry->bv_len,
              __entry->bv_offset)
    
);


//use
DEFINE_EVENT(bio_io_vec_deb, bio_io_vec_deb_loop,

    TP_PROTO(struct bio *bio, int idx),

    TP_ARGS(bio, idx)

);

DEFINE_EVENT_CLASS(uvector_deb,
    
    TP_PROTO(struct iovec *iov),

    TP_ARGS(iov),

    TP_STRUCT__entry(
        __filed(unsigned int, idx)
        __filed(void *, addr)
        __filed(unsigned int, len)
    ),

    TP_fast_assign(
        __entry->idx    = idx,
        __entry->addr   = iov[idx].iov_base,
        __entry->len    = iov[idx].iov_len,
    ),

    TP_print("uvector : idx(%u) addr(%p) len(%u)",
             __entry->idx,
             __entry->iov_base,
             __entry->iov_len)
);


DEFINE_EVENT(uvector_deb, uvector_deb_loop,

    TP_PROTO(struct iovec *iov), 

    TP_ARGS(iov)
);

DEFINE_EVENT(bio_msg_deb, bio_msg_deb_add,
    
    TP_PROTO(char *msg),

	TP_ARGS(msg)

);


DEFINE_EVENT(bio_msg_deb, bio_msg_deb_merge,
    
    TP_PROTO(char *msg),

	TP_ARGS(msg)

);



#endif

#include <trace/define_trace.h>
