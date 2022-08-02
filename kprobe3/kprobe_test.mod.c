#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x22df0971, "module_layout" },
	{ 0x103c1d6d, "unregister_kprobe" },
	{ 0x521e9185, "register_kprobe" },
	{ 0x6b9d9c0f, "proc_remove" },
	{ 0x13cfd03e, "proc_create" },
	{ 0xd215b915, "proc_mkdir" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x619cb7dd, "simple_read_from_buffer" },
	{ 0x91715312, "sprintf" },
	{ 0x20c55ae0, "sscanf" },
	{ 0xbb4f4766, "simple_write_to_buffer" },
	{ 0x27e1a049, "printk" },
	{ 0x1fdc7df2, "_mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "58F9D41F7AED90DFB487C51");
MODULE_INFO(rhelversion, "8.1");
