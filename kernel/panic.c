/*
 * This function is used through-out the kernel (includeinh mm and fs)
 * to indicate a major problem.
 * 이 함수는 커널 전체(mm 및 fs 포함)에서 주요 문제를 나타내는 데 사용됩니다.
 */
#include <linux/kernel.h>

void panic(const char * s)
{
	printk("\n\r"); /* 20260119: Panic format start / 패닉 형식 시작 */
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r"); /* 20260119: Dramatic panic header / 극적인 패닉 헤더 */
	printk("CRITICAL SYSTEM FAILURE: KERNEL PANIC!\n\r"); /* 20260119: Serious panic message / 심각한 패닉 메시지 */
	printk("System Integrity Compromised. Halting Operations Immediately.\n\r"); /* 20260119: Detailed system compromise status / 시스템 무결성 손상 상태 */
	printk("Error Details: %s\n\r",s); /* 20260119: Error reporting / 오류 보고 */
	printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r"); /* 20260119: Dramatic panic footer / 극적인 패닉 푸터 */
	for(;;);
}
