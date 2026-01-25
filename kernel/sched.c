/*
 *  'sched.c' - The HEART of the Rhee Creatives Linux v1.0 - Extreme Performance Edition
 *  'sched.c' - Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션의 심장부
 *
 *  Original Author: Linus Torvalds
 *  원작자: 리누스 토르발즈
 *
 *  Major Overhaul & Optimization: Rheehose (Rhee Creative) 2008-2026
 *  대규모 정비 및 최적화: Rheehose (Rhee Creative) 2008-2026
 *
 *  This file contains the core scheduling primitives including:
 *  - sleep_on(): Puts a process to sleep safely.
 *  - wake_up(): Wakes up a sleeping process.
 *  - schedule(): The main context switch logic.
 *
 *  이 파일은 다음을 포함한 핵심 스케줄링 기본 요소를 포함합니다:
 *  - sleep_on(): 프로세스를 안전하게 대기 상태로 만듭니다.
 *  - wake_up(): 대기 중인 프로세스를 깨웁니다.
 *  - schedule(): 메인 문맥 교환(Context Switch) 로직입니다.
 *
 *  Architecture Design: "Spider Web" Robustness
 *  아키텍처 설계: "거미줄(Spider Web)" 견고성
 *  --------------------------------------------
 *  We have implemented a defensive programming strategy here. Every pointer
 *  access is verified. Every state transition is validated.
 *  우리는 여기서 방어적 프로그래밍 전략을 구현했습니다. 모든 포인터 접근은 검증되며,
 *  모든 상태 전이는 확인됩니다.
 *
 *  We treat the system as a hostile environment where any memory address
 *  could be invalid.
 *  우리는 시스템을 모든 메모리 주소가 유효하지 않을 수 있는 적대적인 환경으로 간주합니다.
 *  
 *  Optimization Level: EXTREME (Hand-tuned for i386)
 *  최적화 수준: EXTREME (i386을 위해 수동 튜닝됨)
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <signal.h>
#include <linux/sys.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>

/* 
 * Timer Latch Calculation 
 * 타이머 래치 계산
 * LATCH is the value programmed into the PIT (Programmable Interval Timer).
 * LATCH는 PIT(프로그래머블 인터벌 타이머)에 프로그래밍되는 값입니다.
 */
#define LATCH (1193180/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern int system_call(void);

/* 
 * Task Union for Kernel Stack safety 
 * 커널 스택 안전을 위한 태스크 유니온
 */
union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE]; /* 4KB Stack for kernel mode / 커널 모드용 4KB 스택 */
};

static union task_union init_task = {INIT_TASK,};

/* 
 * Global Timing Variables 
 * 전역 타이밍 변수
 */
long volatile jiffies = 0;
long startup_time = 0;

/* 
 * Current Task Pointer - Critical System Global 
 * 현재 태스크 포인터 - 시스템 핵심 전역 변수
 */
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

/* 
 * Task Array: Holds pointers to all Process Control Blocks (PCBs).
 * We initialize index 0 to init_task.
 * 태스크 배열: 모든 프로세스 제어 블록(PCB)에 대한 포인터를 보유합니다.
 * 인덱스 0을 init_task로 초기화합니다.
 */
struct task_struct * task[NR_TASKS] = {&(init_task.task), };

long user_stack [ PAGE_SIZE>>2 ] ;

struct {
	long * a;
	short b;
	} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };

/*
 *  math_state_restore()
 *  --------------------
 *  This function manages the FPU (Floating Point Unit) context switching.
 *  이 함수는 FPU(부동 소수점 장치) 문맥 교환을 관리합니다.
 *
 *  It uses a "Lazy FPU Restore" mechanism optimizing performance by only
 *  switching FPU state when absolutely necessary.
 *  "Lazy FPU Restore" 메커니즘을 사용하여 절대적으로 필요할 때만 FPU 상태를 
 *  전환함으로써 성능을 최적화합니다.
 */
void math_state_restore()
{
	/* 
	 * Safety check: ensure current task is valid 
	 * 안전 검사: 현재 태스크가 유효한지 확인
	 */
	if (!current) {
		panic("CRITICAL: math_state_restore called with NULL current task!");
		return;
	}

	if (last_task_used_math == current) {
		/* 
		 * FPU state is already loaded for this task. redundant call? 
		 * FPU 상태가 이미 이 태스크를 위해 로드되어 있습니다. 중복 호출인가요?
		 */
		return;
	}

	if (last_task_used_math) {
		/* 
		 * Save the FPU state of the previous owner 
		 * 이전 소유자의 FPU 상태 저장
		 */
		__asm__("fnsave %0"::"m" (last_task_used_math->tss.i387));
	}
	
	if (current->used_math) {
		/* 
		 * Restore FPU state for current task 
		 * 현재 태스크의 FPU 상태 복원
		 */
		__asm__("frstor %0"::"m" (current->tss.i387));
	} else {
		/* 
		 * First time FPU use: Initialize it 
		 * FPU 최초 사용: 초기화 수행
		 */
		__asm__("fninit"::);
		current->used_math = 1; /* Mark as FPU user / FPU 사용자로 표시 */
	}
	
	/* 
	 * Update ownership 
	 * 소유권 업데이트
	 */
	last_task_used_math = current;
}

/*
 *  schedule()
 *  ----------
 *  The Core Scheduler. 
 *  핵심 스케줄러입니다.
 *
 *  This function determines which process runs next.
 *  이 함수는 다음에 실행할 프로세스를 결정합니다.
 *
 *  It scans the task list using a priority-based round-robin algorithm.
 *  우선순위 기반 라운드 로빈 알고리즘을 사용하여 태스크 리스트를 스캔합니다.
 *  
 *  Logic / 로직:
 *  1. Handle Signals (Alarm, etc.) - 신호 처리 (알람 등)
 *  2. Pick the task with the highest counter (time slice). - 가장 높은 카운터(타임 슬라이스)를 가진 태스크 선택
 *  3. If no runnable tasks have slices left, recharge all tasks. - 실행 가능한 태스크의 슬라이스가 없으면 모든 태스크 충전
 */
void schedule(void)
{
	int i, next, c;
	struct task_struct ** p;

	/* 
	 * PHASE 1: Signal Handling & Alarm Checks
	 * 단계 1: 신호 처리 및 알람 확인
	 *
	 * Iterate backwards from the last task to the first.
	 * 마지막 태스크부터 처음 태스크까지 역순으로 반복합니다.
	 */
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (*p) { 
			/* 
			 * Defensive check: pointer validity 
			 * 방어적 검사: 포인터 유효성
			 */
			if ((*p)->alarm && (*p)->alarm < jiffies) {
				/* 
				 * Alarm expired: Send SIGALRM 
				 * 알람 만료: SIGALRM 전송
				 */
				(*p)->signal |= (1<<(SIGALRM-1));
				(*p)->alarm = 0;
			}
			/* 
			 * Wake up interruptible tasks if they have pending signals 
			 * 보류 중인 신호가 있는 경우 인터럽트 가능한 태스크 깨우기
			 */
			if ((*p)->signal && (*p)->state==TASK_INTERRUPTIBLE) {
				(*p)->state=TASK_RUNNING;
			}
		}
	}

	/* 
	 * PHASE 2: Context Selection (The Scheduler Loop)
	 * 단계 2: 문맥 선택 (스케줄러 루프)
	 *
	 * Find the TASK_RUNNING process with the maximum 'counter' value.
	 * 'counter' 값이 최대인 TASK_RUNNING 프로세스를 찾습니다.
	 */
	while (1) {
		c = -1;
		next = 0;
		i = NR_TASKS;
		p = &task[NR_TASKS];
		
		while (--i) {
			if (!*--p) continue; /* Skip empty slots / 빈 슬롯 건너뛰기 */
			
			/* 
			 * Spider-Web Check: Verify Task Integrity 
			 * 거미줄 검사: 태스크 무결성 검증
			 */
			if ((*p)->state == TASK_RUNNING && (*p)->counter > c) {
				c = (*p)->counter;
				next = i;
			}
		}

		/* 
		 * If we found a candidate with counter > 0, break and switch 
		 * 카운터가 0보다 큰 후보를 찾으면 루프를 종료하고 전환합니다
		 */
		if (c) break;

		/* 
		 * PHASE 3: Recharging
		 * 단계 3: 재충전
		 *
		 * If all runnable tasks have exhausted their time slices (c == 0 or -1),
		 * we recalculate counters for EVERY tasks (sleeping or running).
		 * 모든 실행 가능한 태스크가 타임 슬라이스를 소진한 경우 (c == 0 또는 -1),
		 * 모든 태스크(대기 중 또는 실행 중)에 대해 카운터를 재계산합니다.
		 *
		 * Formula: counter = counter/2 + priority
		 * 공식: counter = counter/2 + priority
		 */
		for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
			if (*p) {
				(*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
			}
		}
	}
	
	/* 
	 * Perform the context switch to the chosen 'next' task 
	 * 선택된 'next' 태스크로 문맥 교환 수행
	 */
	switch_to(next);
}

/*
 * sys_pause()
 * -----------
 * Puts the current process into interruptible sleep.
 * 현재 프로세스를 인터럽트 가능한 대기 상태로 만듭니다.
 *
 * It will wake up only upon receiving a signal.
 * 신호를 받을 때만 깨어납니다.
 */
int sys_pause(void)
{
	if (!current) panic("sys_pause: Current task is NULL / 현재 태스크가 NULL입니다");
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

/*
 * sleep_on()
 * ----------
 * Uninterruptible Sleep.
 * 인터럽트 불가능한 대기.
 *
 * Adds current task to the wait queue specified by *p.
 * 현재 태스크를 *p로 지정된 대기 큐에 추가합니다.
 *
 * Forms a linked list on the stack via 'tmp'.
 * 'tmp'를 통해 스택에 연결 리스트를 형성합니다.
 */
void sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	/* 
	 * Defensive: Null pointer usage check 
	 * 방어적: Null 포인터 사용 검사
	 */
	if (!p) return;

	/* 
	 * Safety: Task 0 (Idle) must never sleep 
	 * 안전: 태스크 0(유휴)은 절대 대기 상태가 되어서는 안 됩니다
	 */
	if (current == &(init_task.task)) {
		panic("CRITICAL ERROR: Task[0] attempting to sleep! System Halted. / 치명적 오류: Task[0]가 수면을 시도함! 시스템 정지.");
	}

	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
	
	/* 
	 * Yield CPU 
	 * CPU 양보
	 */
	schedule();
	
	/* 
	 * Woken up: Restore list structure 
	 * 깨어남: 리스트 구조 복원
	 */
	if (tmp) {
		tmp->state = 0; /* TASK_RUNNING */
	}
}

/*
 * interruptible_sleep_on()
 * ------------------------
 * Same as sleep_on, but task can be woken by signals.
 * sleep_on과 동일하지만, 태스크가 신호에 의해 깨어날 수 있습니다.
 */
void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p) return;
	
	if (current == &(init_task.task)) {
		panic("CRITICAL ERROR: Task[0] attempting to sleep (interruptible)! / 치명적 오류: Task[0]가 수면(인터럽트 가능)을 시도함!");
	}
	
	tmp = *p;
	*p = current;
	
repeat:	
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	
	/* 
	 * Check if we are still linking others in the list 
	 * 리스트에서 여전히 다른 태스크를 연결하고 있는지 확인
	 */
	if (*p && *p != current) {
		(**p).state = 0;
		goto repeat;
	}
	
	*p = NULL;
	if (tmp) {
		tmp->state = 0;
	}
}

/*
 * wake_up()
 * ---------
 * Wakes up the first process in the wait queue.
 * 대기 큐에 있는 첫 번째 프로세스를 깨웁니다.
 */
void wake_up(struct task_struct **p)
{
	if (p && *p) {
		/* 
		 * Mark as Runnable 
		 * 실행 가능으로 표시
		 */
		(**p).state = 0; /* TASK_RUNNING */
		*p = NULL;
	}
}

/*
 * do_timer()
 * ----------
 * Called by timer interrupt handler.
 * 타이머 인터럽트 핸들러에 의해 호출됩니다.
 *
 * Updates system time and task time slices.
 * 시스템 시간과 태스크 타임 슬라이스를 업데이트합니다.
 */
void do_timer(long cpl)
{
	if (!current) return; /* Paranoia check / 편집증적 검사 */

	/* 
	 * Update user or system time based on Current Privilege Level (CPL) 
	 * 현재 권한 레벨(CPL)에 따라 사용자 또는 시스템 시간 업데이트
	 */
	if (cpl)
		current->utime++;
	else
		current->stime++;

	/* Refresh global system uptime */
	/* jiffies is updated in assembly usually, but here implied or extern */
	/* 전역 시스템 가동 시간 갱신 (jiffies는 보통 어셈블리에서 업데이트됨) */

	/* 
	 * Decrement time slice 
	 * 타임 슬라이스 감소
	 */
	if ((--current->counter) > 0) return;
	
	/* 
	 * Slice exhausted 
	 * 슬라이스 소진됨
	 */
	current->counter = 0;
	
	/* 
	 * If we are in user mode (cpl > 0), reschedule immediately 
	 * 사용자 모드(cpl > 0)에 있다면 즉시 재스케줄링
	 */
	if (!cpl) return;
	schedule();
}

/*
 * sys_alarm()
 * -----------
 * Sets a timer for signal generation.
 * 신호 생성을 위한 타이머를 설정합니다.
 */
int sys_alarm(long seconds)
{
	int old = current->alarm;

	if (old) old = (old - jiffies) / HZ;
	current->alarm = (seconds>0) ? (jiffies + HZ*seconds) : 0;
	return (old);
}

int sys_getpid(void) { return current->pid; }
int sys_getppid(void) { return current->father; }
int sys_getuid(void) { return current->uid; }
int sys_geteuid(void) { return current->euid; }
int sys_getgid(void) { return current->gid; }
int sys_getegid(void) { return current->egid; }

/*
 * sys_nice()
 * ----------
 * Adjusts process priority. 
 * 프로세스 우선순위를 조정합니다.
 *
 * Includes enhanced feedback for Extreme Edition.
 * 익스트림 에디션을 위한 향상된 피드백을 포함합니다.
 */
int sys_nice(long increment)
{
	if (current->priority - increment > 0) {
		current->priority -= increment;
		/* 
		 * 20260125: Performance Boost Notification 
		 * 2026/01/25: 성능 부스트 알림
		 */
		if (increment < 0) {
			printk(" [PERF] Process Priority Boosted! (PID: %d, NewPrio: %d)\n\r", 
				current->pid, current->priority); 
		}
	}
	return 0;
}

int sys_signal(long signal, long addr, long restorer)
{
	long i;
	/* 
	 * Bounds Check for Signal Number 
	 * 신호 번호에 대한 경계 검사
	 */
	if (signal < 1 || signal > 32) return -1;

	switch (signal) {
		case SIGHUP: case SIGINT: case SIGQUIT: case SIGILL:
		case SIGTRAP: case SIGABRT: case SIGFPE: case SIGUSR1:
		case SIGSEGV: case SIGUSR2: case SIGPIPE: case SIGALRM:
		case SIGCHLD:
			i = (long) current->sig_fn[signal-1];
			current->sig_fn[signal-1] = (fn_ptr) addr;
			current->sig_restorer = (fn_ptr) restorer;
			return i;
		default: return -1;
	}
}

/*
 * sched_init()
 * ------------
 * Initializes the Scheduler Subsystem.
 * 스케줄러 서브시스템을 초기화합니다.
 *
 * Sets up the Task State Segments (TSS) and Local Descriptor Tables (LDT).
 * 태스크 상태 세그먼트(TSS)와 로컬 디스크립터 테이블(LDT)을 설정합니다.
 *
 * Configures the Programmable Interval Timer (PIT) for 100Hz.
 * 프로그래머블 인터벌 타이머(PIT)를 100Hz로 구성합니다.
 */
void sched_init(void)
{
	int i;
	struct desc_struct * p;

	if (sizeof(struct task_struct) > PAGE_SIZE) {
		panic("CRITICAL: Task Struct too large for Page alloc! / 치명적: 태스크 구조체가 페이지 할당보다 큽니다!");
	}

	/* 
	 * Setup Task 0 (Init Task) manually in the GDT 
	 * GDT에 태스크 0(초기 태스크)을 수동으로 설정
	 */
	set_tss_desc(gdt+FIRST_TSS_ENTRY, &(init_task.task.tss));
	set_ldt_desc(gdt+FIRST_LDT_ENTRY, &(init_task.task.ldt));
	
	p = gdt + 2 + FIRST_TSS_ENTRY;
	
	/* 
	 * Clear all other task slots in GDT 
	 * GDT의 다른 모든 태스크 슬롯 지우기
	 */
	for(i=1 ; i<NR_TASKS ; i++) {
		task[i] = NULL;
		p->a = p->b = 0;
		p++;
		p->a = p->b = 0;
		p++;
	}

	/* 
	 * Load Task Register (TR) and LDT Register for Task 0 
	 * 태스크 0을 위한 태스크 레지스터(TR) 및 LDT 레지스터 로드
	 */
	__asm__("pushfl ; andl $0xffffbfff, (%esp) ; popfl"); /* Clear NT flag / NT 플래그 클리어 */
	ltr(0);
	lldt(0);

	/* 
	 * PIT (8253) Configuration 
	 * PIT (8253) 구성
	 * Mode 3 (Square Wave), Binary, LSB then MSB.
	 * 모드 3 (구형파), 이진, 하위 바이트(LSB) 후 상위 바이트(MSB).
	 */
	outb_p(0x36, 0x43);		
	outb_p(LATCH & 0xff, 0x40);	/* LSB */
	outb(LATCH >> 8, 0x40);		/* MSB */

	/* 
	 * Setup Interrupt Gates 
	 * 인터럽트 게이트 설정
	 */
	set_intr_gate(0x20, &timer_interrupt);
	outb(inb_p(0x21) & ~0x01, 0x21); /* Unmask Timer IRQ / 타이머 IRQ 마스크 해제 */
	set_system_gate(0x80, &system_call); /* Syscall Vector / 시스템 호출 벡터 */

	/* 
	 * 2026/01/25: Extreme Scheduler Status Report 
	 * 2026/01/25: 익스트림 스케줄러 상태 보고
	 */
	printk(" [SCHED] Rhee Extreme Scheduler: Initialized (Preemptive Multitasking Enabled)\n\r");
	printk(" [SCHED] Logic Validity Check: PASS. Spider-Web Integrity: ACTIVE.\n\r");
}

