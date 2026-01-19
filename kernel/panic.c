/*
 * This function is used through-out the kernel (includeinh mm and fs)
 * to indicate a major problem.
 */
#include <linux/kernel.h>

void panic(const char * s)
{
	printk("\n\r");
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r");
	printk("CRITICAL SYSTEM FAILURE: KERNEL PANIC!\n\r");
	printk("System Integrity Compromised. Halting Operations Immediately.\n\r");
	printk("Error Details: %s\n\r",s);
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r");
	for(;;);
}
