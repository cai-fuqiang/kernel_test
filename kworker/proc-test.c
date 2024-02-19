#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/memcontrol.h>
#include <linux/kallsyms.h>
#define NODE "benshushu/my_proc"

static int param = 100;
static struct proc_dir_entry *my_proc;
static struct proc_dir_entry *my_root;
#define KS 3213
static char kstring[KS];        /* should be less sloppy about overflows :) */


static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
       int nbytes = sprintf(kstring, "%d\n", param);
       return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}

int add_num(int a, int b)
{
	int i = 0;
	for (i = 0; i < 100; i++){
		a++;
		b++;
	}
	return a + b;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        ssize_t rc;
	int a = 100;
	int b = 200;
	add_num(a + b, a / b);
        rc = simple_write_to_buffer(kstring, lbuf, ppos, buf, lbuf);
        sscanf(kstring, "%d", &param);
        pr_info("param has been set to %d\n", param);
        return rc;
}

static const struct file_operations my_proc_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

static struct mem_cgroup *(*p__mem_cgroup_iter)(struct mem_cgroup *,
                                   struct mem_cgroup *,
                                   struct mem_cgroup_reclaim_cookie *);
static struct mem_cgroup **p__root_mem_cgroup;
static struct workqueue_struct *loop_memcg_workqueue = NULL;
static struct work_struct loop_memcg_work[10];

struct lruvec *g_lruvec;
static inline struct lruvec *my_mem_cgroup_lruvec(struct mem_cgroup *memcg,
						int node)
{
        struct mem_cgroup_per_node *mz;
        struct lruvec *lruvec;

        if (!memcg)
                memcg = *p__root_mem_cgroup;

        mz = memcg->nodeinfo[node];
        lruvec = &mz->lruvec;
        return lruvec;
}
static void loop_func(int cpu)
{
        struct mem_cgroup *target_memcg = *p__root_mem_cgroup;
        struct mem_cgroup *memcg;

        memcg = p__mem_cgroup_iter(target_memcg, NULL, NULL);
        do {
                struct lruvec *lruvec = my_mem_cgroup_lruvec(memcg, 0);
		g_lruvec = lruvec;
		pr_info("cpu(%d) the memcg(%lx) lruvec(%lx)\n", 
				cpu,
				(unsigned long)memcg,
				(unsigned long)lruvec);
        } while ((memcg = p__mem_cgroup_iter(target_memcg, memcg, NULL)));
}

static void loop_memcg(struct work_struct *work)
{
	//int i = 0;
	int cpu = smp_processor_id();
	pr_info("cpu(%d) ====start\n", cpu);
	loop_func(cpu);
	pr_info("cpu(%d) ====end\n", cpu);
	//for (i = 0; i < 10; i++) {
	//	cpu = smp_processor_id();
	//	msleep(1000);
	//	pr_info("cpu(%d) sleep(%d)\n", cpu, i);
	//}
	return;
}

#define my_get_kallsyms(_symbol_name)   \
        do {  \
                p__##_symbol_name = (typeof(p__##_symbol_name))kallsyms_lookup_name(#_symbol_name);  \
                if (!p__##_symbol_name) {  \
                        pr_info("get %s ptr error\n", #_symbol_name);  \
                        ret = -1;  \
                        goto end;  \
                } \
                pr_info("the %s (%lx) \n", #_symbol_name, (unsigned long)p__##_symbol_name); \
        }while(0)

static int work_init(void)
{
	int ret = 0;
	int i = 0;

	loop_memcg_workqueue = alloc_workqueue("loop_memcg", WQ_MEM_RECLAIM, 0);
	if (!loop_memcg_workqueue) {
		pr_err("alloc workqueue error\n");
		goto err;
	}
	for (i = 0; i < 8; i++) {
		INIT_WORK(&loop_memcg_work[i],  loop_memcg);
		queue_work_on(i, loop_memcg_workqueue, &loop_memcg_work[i]); 
	}
end:
	return 0;
err:
	ret = -1;
	goto end;
}
static void work_destroy(void)
{
	destroy_workqueue(loop_memcg_workqueue);
	loop_memcg_workqueue = NULL;
	return ;
}

static int get_all_kallsym(void)
{
	int ret = 0;

	my_get_kallsyms(mem_cgroup_iter);
	my_get_kallsyms(root_mem_cgroup);
end:
	return ret;
}

static int __init my_init(void)
{

	int ret = 0;
	ret = get_all_kallsym();
	if (ret < 0)
		goto end;

	work_init();
end:
	return 0;
}

static void __exit my_exit(void)
{
	work_destroy();
}
#if 0
static int __init my_init(void)
{
    pr_info("inter the init\n");
    my_root = proc_mkdir("benshushu", NULL);
    if (IS_ERR(my_root)) {
        pr_err("failed create the proc root dir\n"); return -1;
    }
    my_proc = proc_create(NODE, 0, NULL, &my_proc_fops);

    if (IS_ERR(my_proc)) {
        pr_err("failed to create proc file %s\n", NODE);
        return -1;
    }
    return 0;
}
static void __exit my_exit(void)
{
    proc_remove(my_proc);
    pr_info("inter the exit\n");
    return;
}
#endif
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
