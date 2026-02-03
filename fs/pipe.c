/*
 *  linux/fs/pipe.c
 *  (C) 1991 Linus Torvalds
 *  Enhanced & Documented by Rheehose (Rhee Creative) 2008-2026
 *  Rhee Creatives Linux v1.0 - Extreme Performance Edition
 */

#include <signal.h>

#include <linux/sched.h>
#include <linux/mm.h>	/* for get_free_page */
#include <asm/segment.h>

/*
 * read_pipe: Reads data from a pipe.
 * read_pipe: 파이프에서 데이터를 읽어옵니다.
 */
int read_pipe(struct m_inode * inode, char * buf, int count)
{
	char * b=buf;

	while (PIPE_EMPTY(*inode)) {
		wake_up(&inode->i_wait);
		if (inode->i_count != 2) /* are there any writers left? / 남은 쓰기 프로세스가 있는가? */
			return 0;
		sleep_on(&inode->i_wait);
	}
	while (count>0 && !(PIPE_EMPTY(*inode))) {
		count --;
		put_fs_byte(((char *)inode->i_size)[PIPE_TAIL(*inode)],b++);
		INC_PIPE( PIPE_TAIL(*inode) );
	}
	wake_up(&inode->i_wait);
	return b-buf;
}

/*
 * write_pipe: Writes data to a pipe.
 * write_pipe: 파이프에 데이터를 씁니다.
 */
int write_pipe(struct m_inode * inode, char * buf, int count)
{
	char * b=buf;

	wake_up(&inode->i_wait);
	if (inode->i_count != 2) { /* no readers / 읽기 프로세스 없음 */
		current->signal |= (1<<(SIGPIPE-1));
		return -1;
	}
	while (count-->0) {
		while (PIPE_FULL(*inode)) {
			wake_up(&inode->i_wait);
			if (inode->i_count != 2) {
				current->signal |= (1<<(SIGPIPE-1));
				return b-buf;
			}
			sleep_on(&inode->i_wait);
		}
		((char *)inode->i_size)[PIPE_HEAD(*inode)] = get_fs_byte(b++);
		INC_PIPE( PIPE_HEAD(*inode) );
		wake_up(&inode->i_wait);
	}
	wake_up(&inode->i_wait);
	return b-buf;
}

/*
 * sys_pipe: Creates a pipe and returns two file descriptors.
 * sys_pipe: 파이프를 생성하고 두 개의 파일 디스크립터를 반환합니다.
 */
int sys_pipe(unsigned long * fildes)
{
	struct m_inode * inode;
	struct file * f[2];
	int fd[2];
	int i,j;

	j=0;
	for(i=0;j<2 && i<NR_FILE;i++)
		if (!file_table[i].f_count)
			(f[j++]=i+file_table)->f_count++;
	if (j==1)
		f[0]->f_count=0;
	if (j<2)
		return -1;
	j=0;
	for(i=0;j<2 && i<NR_OPEN;i++)
		if (!current->filp[i]) {
			current->filp[ fd[j]=i ] = f[j];
			j++;
		}
	if (j==1)
		current->filp[fd[0]]=NULL;
	if (j<2) {
		f[0]->f_count=f[1]->f_count=0;
		return -1;
	}
	if (!(inode=get_pipe_inode())) {
		current->filp[fd[0]] =
			current->filp[fd[1]] = NULL;
		f[0]->f_count = f[1]->f_count = 0;
		return -1;
	}
	f[0]->f_inode = f[1]->f_inode = inode;
	f[0]->f_pos = f[1]->f_pos = 0;
	f[0]->f_mode = 1;		/* read / 읽기 전용 */
	f[1]->f_mode = 2;		/* write / 쓰기 전용 */

	// printk(" [IPC] Pipe Channel Created (Inode: %d)\n\r", inode->i_num);
	// 20260121: Status check for IPC / IPC 상태 확인

	put_fs_long(fd[0],0+fildes);
	put_fs_long(fd[1],1+fildes);
	return 0;
}
