;
;	boot.s - Rhee Creatives Linux v1.0 - Extreme Performance Edition
;   Original Author: Linus Torvalds
;   Modified by: Rheehose (Rhee Creative) 2008-2026
;
; boot.s is loaded at 0x7c00 by the bios-startup routines, and moves itself
; out of the way to address 0x90000, and jumps there.
; boot.s는 BIOS 시작 루틴에 의해 0x7c00에 로드되며, 스스로를 0x90000 주소로
; 이동시킨 후 그곳으로 점프합니다.
;
; It then loads the system at 0x10000, using BIOS interrupts. Thereafter
; it disables all interrupts, moves the system down to 0x0000, changes
; to protected mode, and calls the start of system. System then must
; RE-initialize the protected mode in it's own tables, and enable
; interrupts as needed.
; 그런 다음 BIOS 인터럽트를 사용하여 시스템을 0x10000에 로드합니다. 그 후
; 모든 인터럽트를 비활성화하고, 시스템을 0x0000으로 이동시키며, 보호 모드(protected mode)로
; 전환하고 시스템의 시작 부분을 호출합니다. 시스템은 자체 테이블에서 보호 모드를
; 다시 초기화하고 필요에 따라 인터럽트를 활성화해야 합니다.
;
; NOTE! currently system is at most 8*65536 bytes long. This should be no
; problem, even in the future. I want to keep it simple. This 512 kB
; kernel size should be enough - in fact more would mean we'd have to move
; not just these start-up routines, but also do something about the cache-
; memory (block IO devices). The area left over in the lower 640 kB is meant
; for these. No other memory is assumed to be "physical", ie all memory
; over 1Mb is demand-paging. All addresses under 1Mb are guaranteed to match
; their physical addresses.
; 주의! 현재 시스템은 최대 8*65536 바이트 길이입니다. 이는 미래에도 문제가 되지
; 않아야 합니다. 저는 단순하게 유지하고 싶습니다. 이 512kB 커널 크기면 충분할 것입니다.
; 사실 더 크다면 이 시작 루틴뿐만 아니라 캐시 메모리(블록 IO 장치)에 대해서도
; 무언가 해야 함을 의미합니다. 하위 640kB에 남은 영역은 이것들을 위한 것입니다.
; 다른 메모리는 "물리적"이라고 가정되지 않습니다. 즉, 1Mb 이상의 모든 메모리는
; 디맨드 페이징(demand-paging)입니다. 1Mb 미만의 모든 주소는 물리적 주소와
; 일치함이 보장됩니다.
;
; NOTE1 abouve is no longer valid in it's entirety. cache-memory is allocated
; above the 1Mb mark as well as below. Otherwise it is mainly correct.
; 주의1 위 내용은 더 이상 전체적으로 유효하지 않습니다. 캐시 메모리는 1Mb 표시
; 위아래로 할당됩니다. 그 외에는 대체로 정확합니다.
;
; NOTE 2! The boot disk type must be set at compile-time, by setting
; the following equ. Having the boot-up procedure hunt for the right
; disk type is severe brain-damage.
; The loader has been made as simple as possible (had to, to get it
; in 512 bytes with the code to move to protected mode), and continuos
; read errors will result in a unbreakable loop. Reboot by hand. It
; loads pretty fast by getting whole sectors at a time whenever possible.
; 주의 2! 부팅 디스크 유형은 다음 equ를 설정하여 컴파일 타임에 설정해야 합니다.
; 부팅 절차가 올바른 디스크 유형을 찾게 하는 것은 심각한 뇌 손상(어리석은 짓)입니다.
; 로더는 가능한 한 간단하게 만들어졌으며(보호 모드로 이동하는 코드를 포함하여
; 512바이트 안에 넣어야 했음), 지속적인 읽기 오류는 깨지지 않는 루프를
; 초래할 것입니다. 수동으로 재부팅하십시오. 가능한 한 번에 전체 섹터를 가져오므로
; 꽤 빨리 로드됩니다.

; 1.44Mb disks:
sectors = 18
; 1.2Mb disks:
; sectors = 15
; 720kB disks:
; sectors = 9

.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

BOOTSEG = 0x07c0
INITSEG = 0x9000
SYSSEG  = 0x1000			; system loaded at 0x10000 (65536).
ENDSEG	= SYSSEG + SYSSIZE

entry start
start:
	mov	ax,#BOOTSEG
	mov	ds,ax
	mov	ax,#INITSEG
	mov	es,ax
	mov	cx,#256
	sub	si,si
	sub	di,di
	rep
	movw
	jmpi	go,INITSEG
go:	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov	sp,#0x400		; arbitrary value >>512 / 임의의 값 >>512

	mov	ah,#0x03	; read cursor pos / 커서 위치 읽기
	xor	bh,bh
	int	0x10
	
	mov	cx,#24
	mov	bx,#0x0007	; page 0, attribute 7 (normal) / 페이지 0, 속성 7 (일반)
	mov	bp,#msg1
	mov	ax,#0x1301	; write string, move cursor / 문자열 쓰기, 커서 이동
	int	0x10

; ok, we've written the message, now
; we want to load the system (at 0x10000)
; 자, 메시지를 작성했습니다. 이제
; 시스템을 로드(0x10000에) 하고 싶습니다.

	mov	ax,#SYSSEG
	mov	es,ax		; segment of 0x010000 / 0x010000 세그먼트
	call	read_it
	call	kill_motor

; if the read went well we get current cursor position ans save it for
; posterity.
; 읽기가 잘 되었다면 현재 커서 위치를 가져와 후세를 위해 저장합니다.

	mov	ah,#0x03	; read cursor pos / 커서 위치 읽기
	xor	bh,bh
	int	0x10		; save it in known place, con_init fetches / 알려진 장소에 저장, con_init가 가져감
	mov	[510],dx	; it from 0x90510. / 0x90510에서.
		
; now we want to move to protected mode ...
; 이제 보호 모드로 이동하고 싶습니다 ...

	cli			; no interrupts allowed ! / 인터럽트 허용 안 됨!

; first we move the system to it's rightful place
; 먼저 시스템을 올바른 위치로 이동합니다.

	mov	ax,#0x0000
	cld			; 'direction'=0, movs moves forward / 방향=0, movs는 앞으로 이동
do_move:
	mov	es,ax		; destination segment / 목적지 세그먼트
	add	ax,#0x1000
	cmp	ax,#0x9000
	jz	end_move
	mov	ds,ax		; source segment / 소스 세그먼트
	sub	di,di
	sub	si,si
	mov 	cx,#0x8000
	rep
	movsw
	j	do_move

; then we load the segment descriptors
; 그런 다음 세그먼트 디스크립터를 로드합니다.

end_move:

	mov	ax,cs		; right, forgot this at first. didn't work :-) / 맞다, 처음에 이걸 잊었더니 작동 안 함 :-)
	mov	ds,ax
	lidt	idt_48		; load idt with 0,0 / idt를 0,0으로 로드
	lgdt	gdt_48		; load gdt with whatever appropriate / 적절한 값으로 gdt 로드

; that was painless, now we enable A20
; 고통 없었음, 이제 A20을 활성화합니다.

	call	empty_8042
	mov	al,#0xD1		; command write / 명령 쓰기
	out	#0x64,al
	call	empty_8042
	mov	al,#0xDF		; A20 on / A20 켜기
	out	#0x60,al
	call	empty_8042

; well, that went ok, I hope. Now we have to reprogram the interrupts :-(
; 음, 잘 됐기를 바랍니다. 이제 인터럽트를 다시 프로그래밍해야 합니다 :-(
; we put them right after the intel-reserved hardware interrupts, at
; int 0x20-0x2F. There they won't mess up anything. Sadly IBM really
; messed this up with the original PC, and they haven't been able to
; rectify it afterwards. Thus the bios puts interrupts at 0x08-0x0f,
; which is used for the internal hardware interrupts as well. We just
; have to reprogram the 8259's, and it isn't fun.
; 우리는 그것들을 인텔 예약 하드웨어 인터럽트 바로 뒤인 int 0x20-0x2F에
; 놓습니다. 거기서는 아무것도 망치지 않을 것입니다. 슬프게도 IBM은 원래 PC에서
; 이것을 정말 망쳤고, 그 후에도 바로잡지 못했습니다. 따라서 BIOS는 인터럽트를
; 0x08-0x0f에 두는데, 이는 내부 하드웨어 인터럽트에도 사용됩니다. 우리는 그저
; 8259를 다시 프로그래밍해야 하는데, 재미있는 일은 아닙니다.

	mov	al,#0x11		; initialization sequence / 초기화 시퀀스
	out	#0x20,al		; send it to 8259A-1 / 8259A-1로 전송
	.word	0x00eb,0x00eb		; jmp $+2, jmp $+2
	out	#0xA0,al		; and to 8259A-2 / 그리고 8259A-2로 전송
	.word	0x00eb,0x00eb
	mov	al,#0x20		; start of hardware int's (0x20) / 하드웨어 인터럽트 시작 (0x20)
	out	#0x21,al
	.word	0x00eb,0x00eb
	mov	al,#0x28		; start of hardware int's 2 (0x28) / 하드웨어 인터럽트 2 시작 (0x28)
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0x04		; 8259-1 is master / 8259-1은 마스터
	out	#0x21,al
	.word	0x00eb,0x00eb
	mov	al,#0x02		; 8259-2 is slave / 8259-2는 슬레이브
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0x01		; 8086 mode for both / 둘 다 8086 모드
	out	#0x21,al
	.word	0x00eb,0x00eb
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0xFF		; mask off all interrupts for now / 지금은 모든 인터럽트 마스크 처리
	out	#0x21,al
	.word	0x00eb,0x00eb
	out	#0xA1,al

; well, that certainly wasn't fun :-(. Hopefully it works, and we don't
; need no steenking BIOS anyway (except for the initial loading :-).
; The BIOS-routine wants lots of unnecessary data, and it's less
; "interesting" anyway. This is how REAL programmers do it.
;
; 음, 확실히 재미없었습니다 :-(. 잘 작동하기를 바라며, 어쨌든 우리는 구린 BIOS가
; 필요 없습니다 (초기 로딩 제외 :-). BIOS 루틴은 불필요한 데이터를 많이 원하고,
; 어쨌든 덜 "흥미롭습니다". 이것이 진짜 프로그래머가 하는 방식입니다.
;
; Well, now's the time to actually move into protected mode. To make
; things as simple as possible, we do no register set-up or anything,
; we let the gnu-compiled 32-bit programs do that. We just jump to
; absolute address 0x00000, in 32-bit protected mode.
;
; 자, 이제 실제로 보호 모드로 이동할 시간입니다. 가능한 한 간단하게 만들기 위해,
; 레지스터 설정이나 다른 어떤 것도 하지 않고, GNU 컴파일된 32비트 프로그램이
; 하도록 둡니다. 우리는 단지 32비트 보호 모드에서 절대 주소 0x00000으로 점프합니다.

	mov	ax,#0x0001	; protected mode (PE) bit / 보호 모드 (PE) 비트
	lmsw	ax		; This is it! / 바로 그거야!
	jmpi	0,8		; jmp offset 0 of segment 8 (cs) / 세그먼트 8 (cs)의 오프셋 0으로 점프

; This routine checks that the keyboard command queue is empty
; No timeout is used - if this hangs there is something wrong with
; the machine, and we probably couldn't proceed anyway.
; 이 루틴은 키보드 명령 큐가 비어 있는지 확인합니다.
; 타임아웃은 사용되지 않습니다 - 멈춘다면 머신에 문제가 있는 것이며,
; 어쨌든 진행할 수 없을 것입니다.
empty_8042:
	.word	0x00eb,0x00eb
	in	al,#0x64	; 8042 status port / 8042 상태 포트
	test	al,#2		; is input buffer full? / 입력 버퍼가 찼는가?
	jnz	empty_8042	; yes - loop / 예 - 루프
	ret

; This routine loads the system at address 0x10000, making sure
; no 64kB boundaries are crossed. We try to load it as fast as
; possible, loading whole tracks whenever we can.
; 이 루틴은 시스템을 주소 0x10000에 로드하며, 64kB 경계를 넘지 않도록
; 확인합니다. 가능한 한 빨리 로드하려고 시도하며, 가능할 때마다 전체 트랙을 로드합니다.
;
; in:	es - starting address segment (normally 0x1000)
; in:	es - 시작 주소 세그먼트 (보통 0x1000)
;
; This routine has to be recompiled to fit another drive type,
; just change the "sectors" variable at the start of the file
; (originally 18, for a 1.44Mb drive)
; 이 루틴은 다른 드라이브 유형에 맞게 다시 컴파일해야 합니다.
; 파일 시작 부분의 "sectors" 변수를 변경하십시오.
; (원래 18, 1.44Mb 드라이브용)
;
sread:	.word 1			; sectors read of current track / 현재 트랙에서 읽은 섹터
head:	.word 0			; current head / 현재 헤드
track:	.word 0			; current track / 현재 트랙
read_it:
	mov ax,es
	test ax,#0x0fff
die:	jne die			; es must be at 64kB boundary / es는 64kB 경계에 있어야 함
	xor bx,bx		; bx is starting address within segment / bx는 세그먼트 내 시작 주소
rp_read:
	mov ax,es
	cmp ax,#ENDSEG		; have we loaded all yet? / 아직 다 로드하지 않았나?
	jb ok1_read
	ret
ok1_read:
	mov ax,#sectors
	sub ax,sread
	mov cx,ax
	shl cx,#9
	add cx,bx
	jnc ok2_read
	je ok2_read
	xor ax,ax
	sub ax,bx
	shr ax,#9
ok2_read:
	call read_track
	mov cx,ax
	add ax,sread
	cmp ax,#sectors
	jne ok3_read
	mov ax,#1
	sub ax,head
	jne ok4_read
	inc track
ok4_read:
	mov head,ax
	xor ax,ax
ok3_read:
	mov sread,ax
	shl cx,#9
	add bx,cx
	jnc rp_read
	mov ax,es
	add ax,#0x1000
	mov es,ax
	xor bx,bx
	jmp rp_read

read_track:
	push ax
	push bx
	push cx
	push dx
	mov dx,track
	mov cx,sread
	inc cx
	mov ch,dl
	mov dx,head
	mov dh,dl
	mov dl,#0
	and dx,#0x0100
	mov ah,#2
	int 0x13
	jc bad_rt
	pop dx
	pop cx
	pop bx
	pop ax
	ret
bad_rt:	mov ax,#0
	mov dx,#0
	int 0x13
	pop dx
	pop cx
	pop bx
	pop ax
	jmp read_track

/*
 * This procedure turns off the floppy drive motor, so
 * that we enter the kernel in a known state, and
 * don't have to worry about it later.
 * 
 * 이 절차는 플로피 드라이브 모터를 꺼서, 알려진 상태로 커널에 진입하고
 * 나중에 걱정할 필요가 없게 합니다.
 */
kill_motor:
	push dx
	mov dx,#0x3f2
	mov al,#0
	outb
	pop dx
	ret

gdt:
	.word	0,0,0,0		; dummy

	.word	0x07FF		; 8Mb - limit=2047 (2048*4096=8Mb)
	.word	0x0000		; base address=0
	.word	0x9A00		; code read/exec / 코드 읽기/실행
	.word	0x00C0		; granularity=4096, 386

	.word	0x07FF		; 8Mb - limit=2047 (2048*4096=8Mb)
	.word	0x0000		; base address=0
	.word	0x9200		; data read/write / 데이터 읽기/쓰기
	.word	0x00C0		; granularity=4096, 386

idt_48:
	.word	0			; idt limit=0
	.word	0,0			; idt base=0L

gdt_48:
	.word	0x800		; gdt limit=2048, 256 GDT entries
	.word	gdt,0x9		; gdt base = 0X9xxxx
	
msg1:
	.byte 13,10
	.ascii "Loading Rhee Creatives Linux v1.0 ..."
	.byte 13,10,13,10

.text
endtext:
.data
enddata:
.bss
endbss:
