//a good crackme tool,usr ptrace to debug the need-crack file
//from tiocsti's solution to yanisto's Tiny Crackme,URL: http://crackmes.de/users/yanisto/tiny_crackme/solutions/tiocsti/browse
//added some comments
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include <linux/user.h>

int handle_ptrace_singlestep(pid_t pid, FILE *fp)
{
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	//在每条指令停止后，记录下需要监控的寄存器的内容
	fprintf(fp, "%08x eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
		regs.eip,
		regs.eax,
		regs.ebx,
		regs.ecx,
		regs.edx,
		regs.esi,
		regs.edi);
	ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
	return 1;
}

int handle_ptrace_syscall(pid_t pid, FILE *fp)
{
	struct user_regs_struct regs;
	//拷贝child 进程的常用或浮点寄存器的数据到&regs中
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
       //下面通过检测寄存器的值，来判断程序执行到什么步骤
	//orig_eax为系统调用号
	//eax为系统调用返回值
	//orig_eax为26，且返回值为-1，则说明子进程识别到被ptrace了
	if(regs.orig_eax == __NR_ptrace && regs.eax == -1)
        {
		fprintf(fp, "identified anti ptrace code...disabling\n");
		//修改返回值eax为0，并返回给子进程
        	regs.eax = 0;
                ptrace(PTRACE_SETREGS, pid, 0, &regs);
        }
	//检测到read系统调用完成了
	else if(regs.orig_eax == __NR_read && regs.ebx == 1 && regs.eax == 4) 
	{
		FILE *outputfile;
		unsigned int i = 0x00200000;
		unsigned int dataword;

		fprintf(fp, "identified sys_read, trying to dump image and switch to single step mode\n");
		//将一个32 bits 的0拷贝到ecx寄存器指向的内存位置
		//ssize_t read(int fd, void *buf, size_t count);
		ptrace(PTRACE_POKEDATA, pid, regs.ecx, 0);

             //下面以二进制方式将内存镜像写入文件
		outputfile = fopen("bmcrack.bin", "wb");
		for(;i < 0x00200320;i+=4)
		{
		//从子进程地址空间的位置i 拷贝一个word，并返回给dataword
		//如果读多了怎么办，这里是不是也应该有容错代码????
			dataword = ptrace(PTRACE_PEEKDATA, pid, i, 0);
			fwrite(&dataword, 4, 1, outputfile);
		}
		//将镜像文件写入完毕
		//作者选择在这个位置开始dump出镜像文件，如果不行就得选其他地方:)
		fclose(outputfile);
		
		//PTRACE_SYSCALL，在执行一条指令后停止
		ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
		return 1;
	}
       //继续执行一次PTRACE_SYSCALL		
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
	return 0;
}

int main(int argc, char **argv)
{
	long ptracehandle;
	pid_t mypid;
	FILE *fp=NULL;
	int state=0;

	mypid = fork();
	switch(mypid)
	{
		case 0:
			// child process
			//执行待监控的对象
			ptrace(PTRACE_TRACEME, 0, 0, 0);
			execl(argv[1], argv[1], NULL);
			_exit(0);
		case -1:
			// error
			perror("fork");
			_exit(-1);
		default:
			fp = fopen("log.txt", "wt");
			if(!fp)
			{
				perror("fopen");
				kill(mypid, 9);
				wait(NULL);
				_exit(-1);
			}
			wait(NULL);
			//通过发送0检查进程是否存在
			if(kill(mypid, 0) == -1)
			{
				fclose(fp);
				exit(0);
			}
			//PTRACE_SYSCALL，在每个系统调用进入/退出时停止
			ptrace(PTRACE_SYSCALL, mypid, 0, 0);
			while(1)
			{
				wait(NULL);
				if(kill(mypid, 0) == -1)
				{
					fclose(fp);
					exit(0);
				}
				if(state == 0)
					//首先，执行去除anti-ptrace的代码
					//然后，检测到read syscall后生成dump文件
					state=handle_ptrace_syscall(mypid, fp);
				else
					//第三，才是开始跟踪read调用后的每一个指令
					state=handle_ptrace_singlestep(mypid, fp);
			}
	}
}

