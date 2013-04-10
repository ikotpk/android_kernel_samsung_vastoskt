/* sec_debug.c
 *
 * Exception handling in kernel by SEC
 *
 * Copyright (c) 2011 Samsung Electronics
 *                http://www.samsung.com/
 */

#ifdef CONFIG_SEC_DEBUG
#include <linux/errno.h>
#include <linux/ctype.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include <linux/file.h>
#include <mach/hardware.h>

#include <mach/msm_iomap.h>
#include <sec_debug.h>

/********************************
 *  Variable
 *********************************/
__used t_sec_arm_core_regsiters sec_core_reg_dump;
__used t_sec_mmu_info           sec_mmu_reg_dump;

/********************************
 *  Function
 *********************************/
/* core reg dump function*/
void sec_get_core_reg_dump(t_sec_arm_core_regsiters *regs)
{
	asm(
		/* we will be in SVC mode when we enter this function.
		* Collect SVC registers along with cmn registers.
		*/
		"str r0, [%0,#0]\n\t"
		"str r1, [%0,#4]\n\t"
		"str r2, [%0,#8]\n\t"
		"str r3, [%0,#12]\n\t"
		"str r4, [%0,#16]\n\t"
		"str r5, [%0,#20]\n\t"
		"str r6, [%0,#24]\n\t"
		"str r7, [%0,#28]\n\t"
		"str r8, [%0,#32]\n\t"
		"str r9, [%0,#36]\n\t"
		"str r10, [%0,#40]\n\t"
		"str r11, [%0,#44]\n\t"
		"str r12, [%0,#48]\n\t"

		/* SVC */
		"str r13, [%0,#52]\n\t"
		"str r14, [%0,#56]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#60]\n\t"

		/* PC and CPSR */
		"sub r1, r15, #0x4\n\t"
		"str r1, [%0,#64]\n\t"
		"mrs r1, cpsr\n\t"
		"str r1, [%0,#68]\n\t"

		/* SYS/USR */
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x1f\n\t"
		"msr cpsr,r1\n\t"

		"str r13, [%0,#72]\n\t"
		"str r14, [%0,#76]\n\t"

		/*FIQ*/
		"mrs r1, cpsr\n\t"
		"and r1,r1,#0xFFFFFFE0\n\t"
		"orr r1,r1,#0x11\n\t"
		"msr cpsr,r1\n\t"

		"str r8, [%0,#80]\n\t"
		"str r9, [%0,#84]\n\t"
		"str r10, [%0,#88]\n\t"
		"str r11, [%0,#92]\n\t"
		"str r12, [%0,#96]\n\t"
		"str r13, [%0,#100]\n\t"
		"str r14, [%0,#104]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#108]\n\t"

		/*IRQ*/
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x12\n\t"
		"msr cpsr,r1\n\t"

		"str r13, [%0,#112]\n\t"
		"str r14, [%0,#116]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#120]\n\t"

		/*MON*/
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x16\n\t"
		"msr cpsr,r1\n\t"

		"str r13, [%0,#124]\n\t"
		"str r14, [%0,#128]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#132]\n\t"

		/*ABT*/
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x17\n\t"
		"msr cpsr,r1\n\t"

		"str r13, [%0,#136]\n\t"
		"str r14, [%0,#140]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#144]\n\t"

		/*UND*/
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x1B\n\t"
		"msr cpsr,r1\n\t"

		"str r13, [%0,#148]\n\t"
		"str r14, [%0,#152]\n\t"
		"mrs r1, spsr\n\t"
		"str r1, [%0,#156]\n\t"

		/* restore to SVC mode */
		"mrs r1, cpsr\n\t"
		"and r1, r1, #0xFFFFFFE0\n\t"
		"orr r1, r1, #0x13\n\t"
		"msr cpsr,r1\n\t"

	:			/* output */
	: "r"(regs)		/* input */
	: "%r1"		/* clobbered register */
	);

	return;
}
EXPORT_SYMBOL(sec_get_core_reg_dump);

int sec_get_mmu_reg_dump(t_sec_mmu_info *mmu_info)
{
	asm("mrc    p15, 0, r1, c1, c0, 0\n\t"	/* SCTLR */
		"str r1, [%0]\n\t"
		"mrc    p15, 0, r1, c2, c0, 0\n\t"	/* TTBR0 */
		"str r1, [%0,#4]\n\t"
		"mrc    p15, 0, r1, c2, c0,1\n\t"	/* TTBR1 */
		"str r1, [%0,#8]\n\t"
		"mrc    p15, 0, r1, c2, c0,2\n\t"	/* TTBCR */
		"str r1, [%0,#12]\n\t"
		"mrc    p15, 0, r1, c3, c0,0\n\t"	/* DACR */
		"str r1, [%0,#16]\n\t"
		"mrc    p15, 0, r1, c5, c0,0\n\t"	/* DFSR */
		"str r1, [%0,#20]\n\t"
		"mrc    p15, 0, r1, c6, c0,0\n\t"	/* DFAR */
		"str r1, [%0,#24]\n\t"
		"mrc    p15, 0, r1, c5, c0,1\n\t"	/* IFSR */
		"str r1, [%0,#28]\n\t"
		"mrc    p15, 0, r1, c6, c0,2\n\t"	/* IFAR */
		"str r1, [%0,#32]\n\t"
		/*Dont populate DAFSR and RAFSR */
		"mrc    p15, 0, r1, c10, c2,0\n\t"	/* PMRRR */
		"str r1, [%0,#44]\n\t"
		"mrc    p15, 0, r1, c10, c2,1\n\t"	/* NMRRR */
		"str r1, [%0,#48]\n\t"
		"mrc    p15, 0, r1, c13, c0,0\n\t"	/* FCSEPID */
		"str r1, [%0,#52]\n\t"
		"mrc    p15, 0, r1, c13, c0,1\n\t"	/* CONTEXT */
		"str r1, [%0,#56]\n\t"
		"mrc    p15, 0, r1, c13, c0,2\n\t"	/* URWTPID */
		"str r1, [%0,#60]\n\t"
		"mrc    p15, 0, r1, c13, c0,3\n\t"	/* UROTPID */
		"str r1, [%0,#64]\n\t"
		"mrc    p15, 0, r1, c13, c0,4\n\t"	/* POTPIDR */
		"str r1, [%0,#68]\n\t"
		:			/* output */
	: "r"(mmu_info)		/* input */
	: "%r1", "memory"	/* clobbered register */
	);
	return 0;
}
EXPORT_SYMBOL(sec_get_mmu_reg_dump);

void sec_save_final_context(void)
{
	sec_get_mmu_reg_dump(&sec_mmu_reg_dump);
	printk(KERN_EMERG"(sec_save_final_context) sec_get_mmu_reg_dump.\n");

	sec_get_core_reg_dump(&sec_core_reg_dump);
	printk(KERN_EMERG "(sec_save_final_context) Final context was saved before the system reset.\n");
}
EXPORT_SYMBOL(sec_save_final_context);

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_DEBUG_PANIC_TEST)
#include <linux/debugfs.h>

static struct dentry *debug_panic_dent;
static spinlock_t debug_panic_spinlock;

struct debug_panic_type {
	void (*func)(void);
	char *desc;
};

void debug_panic_dabort(void)
{
	int *p = 0;
	*p = 0;
}

void debug_panic_pabort(void)
{
	void (*p)(void) = 0;
	p();
}
void debug_panic_lockup(void)
{
	unsigned long flags;
	spin_lock_irqsave(&debug_panic_spinlock, flags);
	while (1)
		;
	spin_unlock_irqrestore(&debug_panic_spinlock, flags);
}

void debug_panic_spinlock_bug(void)
{
	spin_lock(&debug_panic_spinlock);
	spin_lock(&debug_panic_spinlock);

	spin_unlock(&debug_panic_spinlock);
}

void debug_panic_sched_while_atomic(void)
{
	spin_lock(&debug_panic_spinlock);
	msleep(20);
	spin_unlock(&debug_panic_spinlock);
}

void debug_panic_sched_with_irqoff(void)
{
	unsigned long flags;

	raw_local_irq_save(flags);
	msleep(20);
}

struct debug_panic_type debug_panic_scenario[] = {
	[0] = {
		.func = debug_panic_dabort,
		.desc = "data abort\n"
	},
	[1] = {
		.func = debug_panic_pabort,
		.desc = "prefetch abort\n"
	},
	[2] = {
		.func = debug_panic_lockup,
		.desc = "lockup\n"
	},
	[3] = {
		.func = debug_panic_spinlock_bug,
		.desc = "spinlock bug\n"
	},
	[4] = {
		.func = debug_panic_sched_while_atomic,
		.desc = "schedule while atomic\n"
	},
	[5] = {
		.func = debug_panic_sched_with_irqoff,
		.desc = "schedule with irq disabled\n"
	},

};

static int debug_panic_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t debug_panic_read(struct file *file, char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct debug_panic_type *panic_type = file->private_data;
	ssize_t rc;

	rc = simple_read_from_buffer((void __user *) buf, count,\
		ppos, (void *) panic_type->desc,\
		strlen(panic_type->desc));

	return rc;
}
static ssize_t debug_panic_write(struct file *file, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct debug_panic_type *panic_type = file->private_data;

	pr_info("@@ %s %s\n", __func__, panic_type->desc);
	msleep(500);

	panic_type->func();

	return count;
}

static const struct file_operations debug_panic_ops = {
	.open =         debug_panic_open,
	.read =         debug_panic_read,
	.write =        debug_panic_write,
};

#define DEBUG_MAX_FNAME 16
void debug_panic_init(void)
{
	int i;
	char name[DEBUG_MAX_FNAME];

	spin_lock_init(&debug_panic_spinlock);

	debug_panic_dent = debugfs_create_dir("panic", NULL);
	if (IS_ERR(debug_panic_dent)) {
		pr_err("panic debugfs_create_dir fail, error %ld\n",
		       PTR_ERR(debug_panic_dent));
		return;
	}

	for (i = 0; i < ARRAY_SIZE(debug_panic_scenario); i++) {
		snprintf(name, DEBUG_MAX_FNAME-1, "panic-%d", i);
		if (debugfs_create_file(name, 0644, debug_panic_dent,\
			&debug_panic_scenario[i], &debug_panic_ops) == NULL) {
			pr_err("pmic8058 debugfs_create_file %s failed\n",\
				name);
		}
	}
}

static void debug_panic_exit(void)
{
	debugfs_remove_recursive(debug_panic_dent);
}

#else
static void debug_panic_init(void) { }
static void debug_panic_exit(void) { }
#endif
#endif /* CONFIG_SEC_DEBUG */
