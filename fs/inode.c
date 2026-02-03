/*
 *  linux/fs/inode.c
 *  (C) 1991 Linus Torvalds
 *  Enhanced & Documented by Rheehose (Rhee Creative) 2008-2026
 *  Rhee Creatives Linux v1.0 - Extreme Performance Edition
 */

#include <string.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/system.h>

struct m_inode inode_table[NR_INODE]={{0,},};

static void read_inode(struct m_inode * inode);
static void write_inode(struct m_inode * inode);

/*
 * wait_on_inode: Suspends process execution until the specified inode is unlocked.
 * wait_on_inode: 지정된 아이노드의 잠금이 해제될 때까지 프로세스 실행을 중단합니다.
 */
static inline void wait_on_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	sti();
}

/*
 * lock_inode: Acquires a lock on the specified inode, waiting if necessary.
 * lock_inode: 지정된 아이노드에 대한 잠금을 획득하며, 필요한 경우 기다립니다.
 */
static inline void lock_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	inode->i_lock=1;
	sti();
}

/*
 * unlock_inode: Releases the lock on the specified inode and wakes up waiting processes.
 * unlock_inode: 지정된 아이노드의 잠금을 해제하고 대기 중인 프로세스를 깨웁니다.
 */
static inline void unlock_inode(struct m_inode * inode)
{
	inode->i_lock=0;
	wake_up(&inode->i_wait);
}

/*
 * sync_inodes: Synchronizes all dirty inodes in the inode table to the disk.
 * sync_inodes: 아이노드 테이블의 모든 변경된(dirty) 아이노드를 디스크에 동기화합니다.
 */
void sync_inodes(void)
{
	int i;
	struct m_inode * inode;

	inode = 0+inode_table;
	for(i=0 ; i<NR_INODE ; i++,inode++) {
		wait_on_inode(inode);
		if (inode->i_dirt && !inode->i_pipe)
			write_inode(inode);
	}
}

/*
 * _bmap: Internal function to map a logical block number within a file to a physical block number.
 * _bmap: 파일 내의 논리적 블록 번호를 물리적 블록 번호로 매핑하는 내부 함수입니다.
 */
static int _bmap(struct m_inode * inode,int block,int create)
{
	struct buffer_head * bh;
	int i;

	if (block<0)
		panic("_bmap: block<0");
	if (block >= 7+512+512*512)
		panic("_bmap: block>big");
	if (block<7) {
		if (create && !inode->i_zone[block])
			if ((inode->i_zone[block]=new_block(inode->i_dev))) {
				inode->i_ctime=CURRENT_TIME;
				inode->i_dirt=1;
			}
		return inode->i_zone[block];
	}
	block -= 7;
	if (block<512) {
		if (create && !inode->i_zone[7])
			if ((inode->i_zone[7]=new_block(inode->i_dev))) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		if (!inode->i_zone[7])
			return 0;
		if (!(bh = bread(inode->i_dev,inode->i_zone[7])))
			return 0;
		i = ((unsigned short *) (bh->b_data))[block];
		if (create && !i)
			if ((i=new_block(inode->i_dev))) {
				((unsigned short *) (bh->b_data))[block]=i;
				bh->b_dirt=1;
			}
		brelse(bh);
		return i;
	}
	block -= 512;
	if (create && !inode->i_zone[8])
		if ((inode->i_zone[8]=new_block(inode->i_dev))) {
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
		}
	if (!inode->i_zone[8])
		return 0;
	if (!(bh=bread(inode->i_dev,inode->i_zone[8])))
		return 0;
	i = ((unsigned short *)bh->b_data)[block>>9];
	if (create && !i)
		if ((i=new_block(inode->i_dev))) {
			((unsigned short *) (bh->b_data))[block>>9]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
	if (!i)
		return 0;
	if (!(bh=bread(inode->i_dev,i)))
		return 0;
	i = ((unsigned short *)bh->b_data)[block&511];
	if (create && !i)
		if ((i=new_block(inode->i_dev))) {
			((unsigned short *) (bh->b_data))[block&511]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
	return i;
}

int bmap(struct m_inode * inode,int block)
{
	return _bmap(inode,block,0);
}

int create_block(struct m_inode * inode, int block)
{
	return _bmap(inode,block,1);
}

/*
 * iput: Releases an inode, decrementing its reference count and freeing it if necessary.
 * iput: 아이노드를 해제하고 참조 횟수를 감소시키며, 필요한 경우 아이노드를 비웁니다.
 */
void iput(struct m_inode * inode)
{
	if (!inode)
		return;
	wait_on_inode(inode);
	if (!inode->i_count)
		panic("iput: trying to free free inode");
	if (inode->i_pipe) {
		wake_up(&inode->i_wait);
		if (--inode->i_count)
			return;
		free_page(inode->i_size);
		inode->i_count=0;
		inode->i_dirt=0;
		inode->i_pipe=0;
		return;
	}
	if (!inode->i_dev || inode->i_count>1) {
		inode->i_count--;
		return;
	}
repeat:
	if (!inode->i_nlinks) {
		truncate(inode);
		free_inode(inode);
		return;
	}
	if (inode->i_dirt) {
		write_inode(inode);	/* we can sleep - so do again */
		wait_on_inode(inode);
		goto repeat;
	}
	inode->i_count--;
	return;
}

static volatile int last_allocated_inode = 0;

/*
 * get_empty_inode: Finds and returns an unused inode from the inode table.
 * get_empty_inode: 아이노드 테이블에서 사용되지 않는 아이노드를 찾아 반환합니다.
 */
struct m_inode * get_empty_inode(void)
{
	struct m_inode * inode;
	int inr;

	while (1) {
		inode = NULL;
		inr = last_allocated_inode;
		do {
			if (!inode_table[inr].i_count) {
				inode = inr + inode_table;
				break;
			}
			inr++;
			if (inr>=NR_INODE)
				inr=0;
		} while (inr != last_allocated_inode);
		if (!inode) {
			for (inr=0 ; inr<NR_INODE ; inr++)
				printk("%04x: %6d\t",inode_table[inr].i_dev,
					inode_table[inr].i_num);
			panic("No free inodes in mem");
		}
		last_allocated_inode = inr;
		wait_on_inode(inode);
		while (inode->i_dirt) {
			write_inode(inode);
			wait_on_inode(inode);
		}
		if (!inode->i_count)
			break;
	}
	memset(inode,0,sizeof(*inode));
	inode->i_count = 1;
	return inode;
}

/*
 * get_pipe_inode: Returns an inode set up for use as a pipe.
 * get_pipe_inode: 파이프로 사용하도록 설정된 아이노드를 반환합니다.
 */
struct m_inode * get_pipe_inode(void)
{
	struct m_inode * inode;

	if (!(inode = get_empty_inode()))
		return NULL;
	if (!(inode->i_size=get_free_page())) {
		inode->i_count = 0;
		return NULL;
	}
	inode->i_count = 2;	/* sum of readers/writers / 읽기/쓰기 수의 합 */
	PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;
	inode->i_pipe = 1;
	return inode;
}

/*
 * iget: Retrieves an inode with the specified device and inode number.
 * iget: 지정된 장치 및 아이노드 번호를 가진 아이노드를 검색합니다.
 */
struct m_inode * iget(int dev,int nr)
{
	struct m_inode * inode, * empty;

	if (!dev)
		panic("iget with dev==0");
	empty = get_empty_inode();
	inode = inode_table;
	while (inode < NR_INODE+inode_table) {
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode++;
			continue;
		}
		wait_on_inode(inode);
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode = inode_table;
			continue;
		}
		inode->i_count++;
		if (empty)
			iput(empty);
		return inode;
	}
	if (!empty)
		return (NULL);
	inode=empty;
	inode->i_dev = dev;
	inode->i_num = nr;
	read_inode(inode);
	return inode;
}

/*
 * read_inode: Reads the inode data from the disk into memory.
 * read_inode: 디스크의 아이노드 데이터를 메모리로 읽어옵니다.
 */
static void read_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block;

	lock_inode(inode);
	sb=get_super(inode->i_dev);
	block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
		(inode->i_num-1)/INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	*(struct d_inode *)inode =
		((struct d_inode *)bh->b_data)
			[(inode->i_num-1)%INODES_PER_BLOCK];
	brelse(bh);
	unlock_inode(inode);
}

/*
 * write_inode: Writes the inode data from memory back to the disk.
 * write_inode: 메모리의 아이노드 데이터를 디스크로 다시 씁니다.
 */
static void write_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block;

	lock_inode(inode);
	sb=get_super(inode->i_dev);
	block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
		(inode->i_num-1)/INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	((struct d_inode *)bh->b_data)
		[(inode->i_num-1)%INODES_PER_BLOCK] =
			*(struct d_inode *)inode;
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
	unlock_inode(inode);
}
