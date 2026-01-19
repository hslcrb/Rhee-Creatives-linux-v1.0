/*
 * This function is used through-out the kernel (includeinh mm and fs)
 * to indicate a major problem.
 */
#include <linux/kernel.h>

void panic(const char * s)
{
	printk("\n\r"); /* 20260119: Panic format start */
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r"); /* 20260119: Dramatic panic header */
	printk("CRITICAL SYSTEM FAILURE: KERNEL PANIC!\n\r"); /* 20260119: Serious panic message */
	printk("System Integrity Compromised. Halting Operations Immediately.\n\r"); /* 20260119: Detailed system compromise status */
	printk("Error Details: %s\n\r",s); /* 20260119: Error reporting */
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r"); /* 20260119: Dramatic panic footer */
	for(;;);
}
