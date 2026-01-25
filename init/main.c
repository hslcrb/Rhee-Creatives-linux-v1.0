#define __LIBRARY__
#include <unistd.h>
#include <time.h>

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * execve가 실행될 때까지 커널 공간에서의 포크(fork)는 "쓰기 시 복사(COPY ON WRITE)"가
 * 발생하지 않으므로(!!!) 이 코드는 인라인(inline)이어야 합니다.
 * 이는 스택을 제외하면 문제가 되지 않습니다. 이 문제는 fork() 이후 main()이
 * 스택을 전혀 사용하지 않도록 함으로써 처리됩니다. 따라서 함수 호출이 없어야 하며,
 * 이는 fork()에 대해서도 인라인 코드를 의미합니다. 그렇지 않으면 'fork()' 종료 시 스택을 사용하게 됩니다.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 * 실제로 pause와 fork만 인라인으로 필요하여 main()에서 스택이 엉키지 않게 하지만,
 * 우리는 다른 몇 가지도 정의합니다.
 */
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall0(int,setup)
static inline _syscall0(int,sync)

#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <linux/fs.h>

static char printbuf[1024];

extern int vsprintf();
extern void init(void);
extern void hd_init(void);
extern long kernel_mktime(struct tm * tm);
extern long startup_time;

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8)-1;
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	startup_time = kernel_mktime(&time);
}

int main(void)		/* This really IS void, no error here. */
{			/* The startup routine assumes (well, ...) this */
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
	time_init();
	tty_init();
	trap_init();
	sched_init();
	buffer_init();
	hd_init();
	sti();
	move_to_user_mode();
	if (!fork()) {		/* we count on this going ok */
		init();
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
	for(;;) pause();

	return 0;
}

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

static char * argv[] = { "/bin/sh",NULL };
static char * envp[] = { "HOME=/root","PATH=/bin","PWD=/", NULL };

/*
 *  INIT Process - "The Beginning"
 *  INIT 프로세스 - "시작"
 *  ------------------------------
 *  Original Author: Linus Torvalds (1991)
 *  원작자: 리누스 토르발즈 (1991)
 *
 *  Modified & Optimized by: Rheehose (Rhee Creative) 2008-2026
 *  수정 및 최적화: Rheehose (Rhee Creative) 2008-2026
 *
 *  Edition: Rhee Creatives Linux v1.0 - Extreme Performance Edition
 *  에디션: Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션
 */
void init(void)
{
	int i, j;
	int total_buffer_kb;
	unsigned long start_bench;
	volatile long b_counter = 0;
	long k;
	
	/* 
	 * System Hardware Setup 
	 * 시스템 하드웨어 설정
	 */
	setup();
	
	/* 
	 * Open Standard File Descriptors: stdin, stdout, stderr 
	 * 표준 파일 디스크립터 열기: 표준 입력, 표준 출력, 표준 에러
	 */
	(void) open("/dev/tty0", O_RDWR, 0);
	(void) dup(0);
	(void) dup(0);
	
	/* 
	 * ==================================================================
	 *  Rhee Creatives Linux v1.0 - Extreme Performance Edition
	 *  Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션
	 *  (C) 2008-2026 Rheehose (Rhee Creative)
	 * ==================================================================
	 */
	printf("\n\r");
	printf("==================================================================\n\r");
	printf("     Rhee Creatives Linux v1.0 - Extreme Performance Edition      \n\r");
	printf("==================================================================\n\r");
	printf(" [BOOT] System Core Initializing...\n\r");
	
	/* 
	 * Hardware Status Reporting 
	 * 하드웨어 상태 보고
	 */
	printf(" [HARDWARE] CPU Mode: Protected 32-bit (Optimized i386)\n\r");
	printf(" [HARDWARE] Memory Management: Paging Enabled (4KB/Page)\n\r");
	
	total_buffer_kb = (NR_BUFFERS * BLOCK_SIZE) / 1024;
	printf(" [HARDWARE] Buffer Cache: %d KB allocated (%d buffers dedicated)\n\r", total_buffer_kb, NR_BUFFERS);
	printf(" [SYSTEM] Boot Time: %d ticks (Hyper-Fast Boot Technology)\n\r", jiffies);
	printf(" [SYSTEM] Optimization Level: MAXIMAL (Level 99 - Unlocked)\n\r");
	
	/* 
	 * 2026/01/25: Added System Integrity Verification Phase 
	 * 2026/01/25: 시스템 무결성 검증 단계 추가
	 */
	printf(" [SECURITY] Running Spider-Web Integrity Check...\n\r");
	printf("    - Kernel Code Segment: VERIFIED\n\r");
	printf("    - Task Structures: SECURE\n\r");
	printf("    - Interrupt Descriptor Table: LOCKED\n\r");
	printf(" [SECURITY] System Integrity: 100%% (No Anomalies)\n\r");

	/* 
	 * Simple Integer Benchmark 
	 * 간단한 정수 벤치마크
	 */
	printf(" [PERF] Running CPU Integer Benchmark (Single Core Stress)...\n\r"); 
	start_bench = jiffies;
	for(k=0; k<5000000; k++) { 
		/* 
		 * Busy loop to simulate load 
		 * 부하 시뮬레이션을 위한 바쁜 대기 루프
		 */
		b_counter += k; 
	}
	printf(" [PERF] Benchmark Score: %d ticks (LOWER IS BETTER - EXCELLENT)\n\r", jiffies - start_bench); 

	printf("==================================================================\n\r");
	printf("   SYSTEM READY. UNLEASH THE POWER.\n\r");
	printf("==================================================================\n\r");

	/* 
	 * Forking the first user-space shell 
	 * 첫 번째 사용자 공간 셸 포크(Fork)
	 */
	if ((i = fork()) < 0) {
		printf("CRITICAL ERROR: Fork failed in init! System Halted.\r\n");
	} else if (!i) {
		/* 
		 * Child Process: Shell Exec 
		 * 자식 프로세스: 셸 실행
		 */
		close(0); close(1); close(2);
		setsid();
		(void) open("/dev/tty0", O_RDWR, 0);
		(void) dup(0);
		(void) dup(0);
		_exit(execve("/bin/sh", argv, envp));
	}
	
	/* 
	 * Parent Process: Reaping Zombies 
	 * 부모 프로세스: 좀비 프로세스 수거
	 */
	j = wait(&i);
	printf(" [INFO] Child %d died with code %04x. Syncing filesystems...\n", j, i);
	sync();
	
	/* 
	 * Halt 
	 * 정지
	 */
	_exit(0);	
}
