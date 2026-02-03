/*
 *  linux/fs/exec.c
 *
 *  (C) 1991  Linus Torvalds
 *  (C) 2007  Abdel Benamrouche : use elf binary instead of a.out
 *  Enhanced & Documented by Rheehose (Rhee Creative) 2008-2026
 *  Rhee Creatives Linux v1.0 - Extreme Performance Edition
 */

#include <errno.h>
#include <sys/stat.h>
#include <elf.h>

#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/segment.h>

extern int sys_exit(int exit_code);
extern int sys_close(int fd);

/*
 * MAX_ARG_PAGES defines the number of pages allocated for arguments
 * and envelope for the new program. 32 should suffice, this gives
 * a maximum env+arg of 128kB !
 *
 * MAX_ARG_PAGES는 새 프로그램을 위한 인자 및 환경 변수에 할당된 페이지 수를 정의합니다.
 * 32개면 충분하며, 이는 최대 128kB의 env+arg를 제공합니다!
 */
#define MAX_ARG_PAGES 32

inline void cp_block(const void * from,void * to, int size)
{
int d0,d1,d2;
__asm__ __volatile("pushl $0x10\n\t"
	"pushl $0x17\n\t"
	"pop %%es\n\t"
	"cld\n\t"
	"rep\n\t"
	"movsl\n\t"
	"pop %%es"
	:"=&c" (d0), "=&S" (d1), "=&D" (d2)
	:"0" (size/4),"1" (from),"2" (to)
	:"memory");
}

typedef struct
{
	unsigned long b_entry;
	unsigned long b_size;				//file size
}bin_section;


#include <string.h>

/*
 * create_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the "stack", returning the new stack pointer value.
 *
 * create_tables()는 새 사용자 메모리에서 환경 변수 및 인자 문자열을 파싱하고
 * 이들로부터 포인터 테이블을 생성하며, 그 주소를 "스택"에 넣고
 * 새 스택 포인터 값을 반환합니다.
 */
static unsigned long * create_tables(char * p,int argc,int envc)
{
	unsigned long *argv,*envp;
	unsigned long * sp;

	sp = (unsigned long *) (0xfffffffc & (unsigned long) p);
	sp -= envc+1;
	envp = sp;
	sp -= argc+1;
	argv = sp;
	//put_fs_long((unsigned long)envp,--sp);
	//put_fs_long((unsigned long)argv,--sp);
	put_fs_long((unsigned long)argc,--sp);
	while (argc-->0) {
		put_fs_long((unsigned long) p,argv++);
		while (get_fs_byte(p++)) /* nothing */ ;
	}
	put_fs_long(0,argv);
	while (envc-->0) {
		put_fs_long((unsigned long) p,envp++);
		while (get_fs_byte(p++)) /* nothing */ ;
	}
	put_fs_long(0,envp);
	return sp;
}

/*
 * count() counts the number of arguments/envelopes
 * count()는 인자/환경 변수의 개수를 셉니다.
 */
static int count(char ** argv)
{
	int i=0;
	char ** tmp;

	if ((tmp = argv))
		while (get_fs_long((unsigned long *) (tmp++)))
			i++;

	return i;
}

/*
 * 'copy_string()' copies argument/envelope strings from user
 * memory to free pages in kernel mem. These are in a format ready
 * to be put directly into the top of new user memory.
 *
 * 'copy_string()'은 사용자 메모리에서 커널 메모리의 빈 페이지로 인자/환경 변수
 * 문자열을 복사합니다. 이들은 새 사용자 메모리의 상단에 직접 넣을 수 있는
 * 준비된 형식입니다.
 */
static unsigned long copy_strings(int argc,char ** argv,unsigned long *page,
		unsigned long p)
{
	int len,i;
	char *tmp;

	while (argc-- > 0) {
		if (!(tmp = (char *)get_fs_long(((unsigned long *) argv)+argc)))
			panic("argc is wrong");
		len=0;		/* remember zero-padding / 제로 패딩 기억 */
		do {
			len++;
		} while (get_fs_byte(tmp++));
		if (p-len < 0)		/* this shouldn't happen - 128kB / 이런 일은 없어야 함 - 128kB */
			return 0;
		i = ((unsigned) (p-len)) >> 12;
		while (i<MAX_ARG_PAGES && !page[i]) {
			if (!(page[i]=get_free_page()))
				return 0;
			i++;
		}
		do {
			--p;
			if (!page[p/PAGE_SIZE])
				panic("nonexistent page in exec.c");
			((char *) page[p/PAGE_SIZE])[p%PAGE_SIZE] =
				get_fs_byte(--tmp);
		} while (--len);
	}
	return p;
}

static unsigned long change_ldt(unsigned long text_size,unsigned long * page)
{
	unsigned long code_limit,data_limit,code_base,data_base;
	int i;

	code_limit = text_size+PAGE_SIZE -1;
	code_limit &= 0xFFFFF000;
	data_limit = 0x4000000;
	code_base = get_base(current->ldt[1]);
	data_base = code_base;
	set_base(current->ldt[1],code_base);
	set_limit(current->ldt[1],code_limit);
	set_base(current->ldt[2],data_base);
	set_limit(current->ldt[2],data_limit);
/* make sure fs points to the NEW data segment */
/* fs가 새로운 데이터 세그먼트를 가리키도록 확인 */
	__asm__("pushl $0x17\n\tpop %%fs"::);
	data_base += data_limit;
	for (i=MAX_ARG_PAGES-1 ; i>=0 ; i--) {
		data_base -= PAGE_SIZE;
		if (page[i])
			put_page(page[i],data_base);
	}
	return data_limit;
}

/*
 * return 1 if ex if a valid elf executable
 * ex가 유효한 elf 실행 파일이면 1을 반환
 */
int is_valid_elf(Elf32_Ehdr* ex)
{
	if (ex->e_ident[EI_MAG0]!=ELFMAG0 || ex->e_ident[EI_MAG1]!=ELFMAG1 ||
		ex->e_ident[EI_MAG2]!=ELFMAG2 || ex->e_ident[EI_MAG3]!=ELFMAG3){
		printk(" [ELF] Not ELF Format (Magic Mismatch)\n\r");
		/* [ELF] ELF 형식이 아님 (매직 미스매치) */
		return 0;	/*not elf format */
	}

	if (ex->e_type != ET_EXEC || ex->e_machine != EM_386){
		printk(" [ELF] Bad ELF Binary (Type/Machine Error)\n\r");
		/* [ELF] 잘못된 ELF 바이너리 (타입/머신 오류) */
		return 0;
	}

	return 1;
}

/*
 * bread(dev,ind) is an array of block index
 * So we have to get from this array our wanted index ( variable block)
 */
struct buffer_head* read_file_block_ind(int dev,int ind,int block_num)
{
	struct buffer_head * bh=NULL,*ih;
	unsigned short block;

	if (!(ih=bread(dev,ind)))
	{
		printk(" [FS] Bad Block Table\n\r");
		/* [FS] 잘못된 블록 테이블 */
		return NULL;
	}

	if ((block = *((unsigned short *) (ih->b_data) + block_num))){
		if (!(bh=bread(dev,block))) {
			brelse(ih);
			printk(" [FS] Bad Block File\n\r");
			/* [FS] 잘못된 블록 파일 */
			return NULL;
		}
	}


	brelse(ih);
	return bh;
}

/*
 * if block_num=5, read file at position 5*1024.
 * this work only for minix 1 fs 
 * block_num=5이면 파일의 5*1024 위치를 읽습니다.
 * 이 기능은 minix 1 fs에서만 작동합니다.
 */
struct buffer_head* read_file_block(struct m_inode * inode,int block_num)
{
	struct buffer_head * dind;
	int i;
	unsigned short table;

	 /* 7 1st block can be read directly / 처음 7개 블록은 직접 읽을 수 있음 */
	if (block_num<=6)
	{
		return bread(inode->i_dev,inode->i_zone[block_num]);
	}

	block_num-=7;

	if (block_num<512)
	{
		/* read block at i_zone[7] is an array of block index */
		/* i_zone[7]에서 읽은 블록은 블록 인덱스의 배열임 */
		return read_file_block_ind(inode->i_dev,inode->i_zone[7],block_num);
	}

	block_num-=512;

	if (block_num>512*512)
	{
		panic("Impossibly long executable");
		return NULL;	//just to avoid warning compilation
	}

	/* i_zone[8] => array of array block index / i_zone[8] => 블록 인덱스 배열의 배열 */
	if (!(i=inode->i_zone[8]))
		return NULL;
	if (!(dind = bread(inode->i_dev,i)))
		return NULL;
	
	table = *((unsigned short *) dind->b_data+(block_num/512));
	brelse(dind);
	if (block_num>=512)
		block_num-=(block_num-1)*512;
	
	return read_file_block_ind(inode->i_dev,table,block_num);
}


/*  
 *  read an area into %fs:mem.
 *  영역을 %fs:mem으로 읽어옵니다.
 */
int copy_section(struct m_inode * inode,Elf32_Off from, Elf32_Addr dest,Elf32_Word size)
{
	struct buffer_head * bh;
	int block_num=from/BLOCK_SIZE;
	int block_offset;			//only for 1st block
	int cp_size;				

	//read fist block / 첫 번째 블록 읽기
	block_offset=from%BLOCK_SIZE;
	bh=read_file_block(inode,block_num);
	if (!bh) return -1;
	cp_size=(size<BLOCK_SIZE-block_offset)?size:BLOCK_SIZE-block_offset;
	//note we add 3 to size to get it aligned to word boundary
	cp_block(bh->b_data+block_offset,(void*)dest,cp_size+3);
	brelse(bh);
	dest+=cp_size;
	size-=cp_size;
	block_num++;

	//read others blocks / 다른 블록 읽기
	while(size)
	{
		bh=read_file_block(inode,block_num);
		if (!bh) return -1;
		cp_size=(size<BLOCK_SIZE)?size:BLOCK_SIZE;
		cp_block(bh->b_data,(void*)dest,cp_size+3);
		block_num++;
		size-=cp_size;
		dest+=cp_size;
		brelse(bh);
	};

	return 0;
}

inline int create_bss_section(Elf32_Addr dest,Elf32_Word size)
{
	while (size--)
		put_fs_byte(0,(char *)dest++);
	return 0;
}

/*
 * in linux 0.01 only a.out binary format was supported
 * Today it's hard to compile some programs (like bach) in a.out format
 * So this version of linux 0.01 support elf binary.
 * nb : there is no support of shared library !
 * 
 * linux 0.01에서는 a.out 바이너리 형식만 지원되었습니다.
 * 오늘날 (bash 같은) 일부 프로그램을 a.out 형식으로 컴파일하기는 어렵습니다.
 * 그래서 이 버전의 linux 0.01은 elf 바이너리를 지원합니다.
 * 참고: 공유 라이브러리는 지원하지 않습니다!
 */
int load_elf_binary(struct m_inode *inode, struct buffer_head* bh,
					 bin_section* bs)
{
	Elf32_Ehdr ex;
	Elf32_Shdr sect;
	long nsect;
	int lb=0;
	struct buffer_head* bht=NULL;

	ex = *((Elf32_Ehdr *) bh->b_data);	/* read exec-header / 실행 헤더 읽기 */

	/* check header / 헤더 확인 */
	if (!is_valid_elf(&ex)) {
		return -1;
	}

	bs->b_entry=ex.e_entry;
	nsect=ex.e_shnum;
	bs->b_size=ex.e_ehsize+nsect*ex.e_shentsize;
	
	if (nsect<=1){
		printk(" [ELF] Bad Nb of Sections: %d\n\r",nsect);
		/* [ELF] 잘못된 섹션 수: %d */
		return -1;
	}
	
	while(nsect--){
	
		/* load block where is our table section / 테이블 섹션이 있는 블록 로드 */
		if ((ex.e_shoff + nsect * ex.e_shentsize)/BLOCK_SIZE != lb)
		{
			printk("");	// gcc  bug : removing this line = gcc not happy !!!
			lb = (ex.e_shoff + nsect * ex.e_shentsize)/BLOCK_SIZE;
			if (bht) brelse(bht);
			bht=read_file_block(inode,lb);
			if (!bht) return -1;
		}

		/* copy this section fo %fs:mem / 이 섹션을 %fs:mem으로 복사 */
		sect=*((Elf32_Shdr *)(bht->b_data + (ex.e_shoff + 
				nsect * ex.e_shentsize)%BLOCK_SIZE ));
	
		if (!sect.sh_size || !sect.sh_addr) continue;
		
		switch(sect.sh_type)
		{
			case SHT_PROGBITS:
				if (copy_section(inode,sect.sh_offset,sect.sh_addr,
					sect.sh_size))
				{
					if (bht) brelse(bht);
					return -1;
				}
				break;
			
			case SHT_NOBITS:	  
				create_bss_section(sect.sh_addr,sect.sh_size);
				break;

			default:
			continue;	  
		}

		/* find binary size / 바이너리 크기 찾기 */
		if (bs->b_size<sect.sh_addr + sect.sh_size)
			bs->b_size=sect.sh_addr + sect.sh_size;	
	};

	if (bht) brelse(bht);
	return 0;
}

/*
 * 'do_execve()' executes a new program.
 * 'do_execve()'는 새 프로그램을 실행합니다.
 */
int do_execve(unsigned long * eip,long tmp,char * filename,
	char ** argv, char ** envp)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	unsigned long page[MAX_ARG_PAGES];
	int i,argc,envc;
	unsigned long p;
	bin_section bs;

	if ((0xffff & eip[1]) != 0x000f)
		panic("execve called from supervisor mode");
	for (i=0 ; i<MAX_ARG_PAGES ; i++)	/* clear page-table / 페이지 테이블 클리어 */
		page[i]=0;
	if (!(inode=namei(filename)))		/* get executables inode / 실행 파일의 아이노드 가져오기 */
		return -ENOENT;
	if (!S_ISREG(inode->i_mode)) {		/* must be regular file / 일반 파일이어야 함 */
		iput(inode);
		return -EACCES;
	}
	i = inode->i_mode;
	if (current->uid && current->euid) {
		if (current->euid == inode->i_uid)
			i >>= 6;
		else if (current->egid == inode->i_gid)
			i >>= 3;
	} else if (i & 0111)
		i=1;
	if (!(i & 1)) {
		iput(inode);
		return -ENOEXEC;
	}
	if (!(bh = bread(inode->i_dev,inode->i_zone[0]))) {
		iput(inode);
		return -EACCES;
	}

	argc = count(argv);
	envc = count(envp);

	printk(" [EXEC] Loading: %s (Args: %d, Envs: %d)\n\r", filename, argc, envc);
	/* [EXEC] 로딩 중: %s (인자: %d, 환경변수: %d) */

	p = copy_strings(envc,envp,page,PAGE_SIZE*MAX_ARG_PAGES);
	p = copy_strings(argc,argv,page,p);
	if (!p) {
		for (i=0 ; i<MAX_ARG_PAGES ; i++)
			free_page(page[i]);
		iput(inode);
		return -1;
	}
/* OK, This is the point of no return / 자, 이제 되돌릴 수 없는 지점입니다 */
	for (i=0 ; i<32 ; i++)
		current->sig_fn[i] = NULL;
	for (i=0 ; i<NR_OPEN ; i++)
		if ((current->close_on_exec>>i)&1)
			sys_close(i);
	current->close_on_exec = 0;

	free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
	free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
	
	if (load_elf_binary(inode,bh,&bs)){
		brelse(bh);
		iput(inode);
		return -EACCES;
	}
	brelse(bh);

	if (last_task_used_math == current)
		last_task_used_math = NULL;
	current->used_math = 0;
	p += change_ldt(bs.b_size,page)-MAX_ARG_PAGES*PAGE_SIZE;
	p = (unsigned long) create_tables((char *)p,argc,envc);
	current->brk = bs.b_size;
	current->end_data = bs.b_size;
	current->end_code = bs.b_size;
	current->start_stack = p & 0xfffff000;
	iput(inode);

	i = bs.b_size;
	while (i&0xfff)
		put_fs_byte(0,(char *) (i++));
	
	eip[0] = bs.b_entry;		/* eip, magic happens :-) / eip, 마법이 일어납니다 :-) */
	eip[3] = p;					/* stack pointer / 스택 포인터 */

	return 0;
}
