/*
 *  head.s contains the 32-bit startup code.
 *  head.s는 32비트 시작 코드를 포함합니다.
 *
 *  Original Author: Linus Torvalds
 *  Modified by: Rheehose (Rhee Creative) 2008-2026
 *  Rhee Creatives Linux v1.0 - Extreme Performance Edition
 *
 * NOTE!!! Startup happens at absolute address 0x00000000, which is also where
 * the page directory will exist. The startup code will be overwritten by
 * the page directory.
 *
 * 주의!!! 시작은 절대 주소 0x00000000에서 발생하며, 페이지 디렉터리도 여기에 존재합니다.
 * 시작 코드는 페이지 디렉터리에 의해 덮어쓰여집니다.
 */
.code32
.text
.globl idt,gdt,pg_dir,startup_32
pg_dir:
startup_32:
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss stack_start,%esp
	call setup_idt
	call setup_gdt
	movl $0x10,%eax		# reload all the segment registers
				# after changing gdt. CS was already
				# reloaded in 'setup_gdt'
				# gdt 변경 후 모든 세그먼트 레지스터를 다시 로드합니다.
				# CS는 이미 'setup_gdt'에서 다시 로드되었습니다.
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss stack_start,%esp
	xorl %eax,%eax
1:	incl %eax		# check that A20 really IS enabled
				# A20이 정말로 활성화되었는지 확인합니다.
	movl %eax,0x000000
	cmpl %eax,0x100000
	je 1b
	movl %cr0,%eax		# check math chip / 수학 칩 확인
	andl $0x80000011,%eax	# Save PG,ET,PE
	testl $0x10,%eax
	jne 1f			# ET is set - 387 is present
				# ET가 설정됨 - 387이 존재합니다.
	orl $4,%eax		# else set emulate bit / 그렇지 않으면 에뮬레이트 비트 설정
1:	movl %eax,%cr0
	jmp after_page_tables

/*
 *  setup_idt
 *
 *  sets up a idt with 256 entries pointing to
 *  ignore_int, interrupt gates. It then loads
 *  idt. Everything that wants to install itself
 *  in the idt-table may do so themselves. Interrupts
 *  are enabled elsewhere, when we can be relatively
 *  sure everything is ok. This routine will be over-
 *  written by the page tables.
 *
 *  256개 엔트리를 가진 idt를 설정하며, 각 엔트리는 ignore_int(인터럽트 게이트)를
 *  가리킵니다. 그런 다음 idt를 로드합니다. idt 테이블에 자신을 설치하려는 모든 것은
 *  스스로 그렇게 할 수 있습니다. 인터럽트는 모든 것이 괜찮다고 비교적 확신할 수 있을 때
 *  다른 곳에서 활성화됩니다. 이 루틴은 페이지 테이블에 의해 덮어쓰여질 것입니다.
 */
setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	lidt idt_descr
	ret

/*
 *  setup_gdt
 *
 *  This routines sets up a new gdt and loads it.
 *  Only two entries are currently built, the same
 *  ones that were built in init.s. The routine
 *  is VERY complicated at two whole lines, so this
 *  rather long comment is certainly needed :-).
 *  This routine will beoverwritten by the page tables.
 *
 *  이 루틴은 새로운 gdt를 설정하고 로드합니다.
 *  현재 두 개의 엔트리만 구축되며, init.s에서 구축된 것과 동일합니다.
 *  이 루틴은 전체 두 줄로 매우 복잡하므로, 이 다소 긴 주석이 확실히 필요합니다 :-).
 *  이 루틴은 페이지 테이블에 의해 덮어쓰여질 것입니다.
 */
setup_gdt:
	lgdt gdt_descr
	ret

.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:		# This is not used yet, but if you
		# want to expand past 8 Mb, you'll have
		# to use it.
		# 이것은 아직 사용되지 않지만, 8Mb를 초과하여
		# 확장하려면 사용해야 합니다.

.org 0x4000
after_page_tables:
	pushl $0		# These are the parameters to main :-)
				# 이것들은 main의 매개변수입니다 :-)
	pushl $0
	pushl $0
	pushl $L6		# return address for main, if it decides to.
				# main의 반환 주소 (결정하는 경우).
	pushl $main
	jmp setup_paging
L6:
	jmp L6			# main should never return here, but
				# just in case, we know what happens.
				# main은 여기로 절대 반환되지 않아야 하지만,
				# 만약을 위해 무슨 일이 일어나는지 알고 있습니다.

/* This is the default interrupt "handler" :-) */
/* 이것은 기본 인터럽트 "핸들러"입니다 :-) */
.align 2
ignore_int:
	incb 0xb8000+160		# put something on the screen
					# 화면에 무언가를 표시합니다.
	movb $2,0xb8000+161		# so that we know something
					# 무언가가 일어났음을 알기 위해
	iret				# happened / 발생했습니다.


/*
 * Setup_paging
 *
 * This routine sets up paging by setting the page bit
 * in cr0. The page tables are set up, identity-mapping
 * the first 8MB. The pager assumes that no illegal
 * addresses are produced (ie >4Mb on a 4Mb machine).
 *
 * 이 루틴은 cr0의 페이지 비트를 설정하여 페이징을 설정합니다.
 * 페이지 테이블이 설정되어 첫 8MB를 동일 매핑(identity-mapping)합니다.
 * 페이저는 불법적인 주소가 생성되지 않는다고 가정합니다 
 * (예: 4Mb 머신에서 >4Mb).
 *
 * NOTE! Although all physical memory should be identity
 * mapped by this routine, only the kernel page functions
 * use the >1Mb addresses directly. All "normal" functions
 * use just the lower 1Mb, or the local data space, which
 * will be mapped to some other place - mm keeps track of
 * that.
 *
 * 주의! 모든 물리적 메모리가 이 루틴에 의해 동일 매핑되어야 하지만,
 * 커널 페이지 함수만 >1Mb 주소를 직접 사용합니다. 모든 "일반" 함수는
 * 낮은 1Mb만 사용하거나, 다른 곳에 매핑될 로컬 데이터 공간을 사용합니다 -
 * mm이 이를 추적합니다.
 *
 * For those with more memory than 8 Mb - tough luck. I've
 * not got it, why should you :-) The source is here. Change
 * it. (Seriously - it shouldn't be too difficult. Mostly
 * change some constants etc. I left it at 8Mb, as my machine
 * even cannot be extended past that (ok, but it was cheap :-)
 * I've tried to show which constants to change by having
 * some kind of marker at them (search for "8Mb"), but I
 * won't guarantee that's all :-( )
 *
 * 8Mb보다 많은 메모리를 가진 사람들을 위해 - 안됐습니다. 저는 그것을 가지고
 * 있지 않은데, 왜 당신이 가져야 합니까 :-) 소스는 여기 있습니다. 변경하세요.
 * (진지하게 - 너무 어렵지 않아야 합니다. 대부분 일부 상수 등을 변경하면 됩니다.
 * 제 머신은 그것을 넘어서 확장할 수 없기 때문에 8Mb로 남겨 두었습니다
 * (좋아요, 하지만 싸게 샀어요 :-). 어떤 상수를 변경할지 표시하려고 노력했습니다
 * (일종의 마커를 사용하여 - "8Mb" 검색), 하지만 그것이 전부라고 보장하지는 않습니다 :-( )
 */
.align 2
setup_paging:
	movl $1024*3,%ecx
	xorl %eax,%eax
	xorl %edi,%edi			/* pg_dir is at 0x000 */
					/* pg_dir는 0x000에 있습니다 */
	cld;rep;stosl
	movl $pg0+7,pg_dir		/* set present bit/user r/w */
					/* present 비트/사용자 r/w 설정 */
	movl $pg1+7,pg_dir+4		/*  --------- " " --------- */
	movl $pg1+4092,%edi
	movl $0x7ff007,%eax		/*  8Mb - 4096 + 7 (r/w user,p) */
	std
1:	stosl			/* fill pages backwards - more efficient :-) */
				/* 페이지를 거꾸로 채움 - 더 효율적 :-) */
	subl $0x1000,%eax
	jge 1b
	xorl %eax,%eax		/* pg_dir is at 0x0000 */
				/* pg_dir는 0x0000에 있습니다 */
	movl %eax,%cr3		/* cr3 - page directory start */
				/* cr3 - 페이지 디렉터리 시작 */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		/* set paging (PG) bit */
				/* 페이징 (PG) 비트 설정 */
	ret			/* this also flushes prefetch-queue */
				/* 이것은 또한 prefetch-queue를 플러시합니다 */

.align 2
.word 0
idt_descr:
	.word 256*8-1		# idt contains 256 entries
				# idt는 256개의 엔트리를 포함합니다
	.long idt
.align 2
.word 0
gdt_descr:
	.word 256*8-1		# so does gdt (not that that's any
				# gdt도 그렇습니다 (그게 무슨
	.long gdt		# magic number, but it works for me :^)
				# 매직 넘버는 아니지만, 저에게는 작동합니다 :^)

	.align 8
idt:	.fill 256,8,0		# idt is uninitialized
				# idt는 초기화되지 않았습니다

gdt:	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x00c09a00000007ff	/* 8Mb */
	.quad 0x00c09200000007ff	/* 8Mb */
	.quad 0x0000000000000000	/* TEMPORARY - don't use */
					/* 임시 - 사용하지 마세요 */
	.fill 252,8,0			/* space for LDT's and TSS's etc */
					/* LDT와 TSS 등을 위한 공간 */
