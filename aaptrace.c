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
	//��ÿ��ָ��ֹͣ�󣬼�¼����Ҫ��صļĴ���������
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
	//����child ���̵ĳ��û򸡵�Ĵ��������ݵ�&regs��
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
       //����ͨ�����Ĵ�����ֵ�����жϳ���ִ�е�ʲô����
	//orig_eaxΪϵͳ���ú�
	//eaxΪϵͳ���÷���ֵ
	//orig_eaxΪ26���ҷ���ֵΪ-1����˵���ӽ���ʶ�𵽱�ptrace��
	if(regs.orig_eax == __NR_ptrace && regs.eax == -1)
        {
		fprintf(fp, "identified anti ptrace code...disabling\n");
		//�޸ķ���ֵeaxΪ0�������ظ��ӽ���
        	regs.eax = 0;
                ptrace(PTRACE_SETREGS, pid, 0, &regs);
        }
	//��⵽readϵͳ���������
	else if(regs.orig_eax == __NR_read && regs.ebx == 1 && regs.eax == 4) 
	{
		FILE *outputfile;
		unsigned int i = 0x00200000;
		unsigned int dataword;

		fprintf(fp, "identified sys_read, trying to dump image and switch to single step mode\n");
		//��һ��32 bits ��0������ecx�Ĵ���ָ����ڴ�λ��
		//ssize_t read(int fd, void *buf, size_t count);
		ptrace(PTRACE_POKEDATA, pid, regs.ecx, 0);

             //�����Զ����Ʒ�ʽ���ڴ澵��д���ļ�
		outputfile = fopen("bmcrack.bin", "wb");
		for(;i < 0x00200320;i+=4)
		{
		//���ӽ��̵�ַ�ռ��λ��i ����һ��word�������ظ�dataword
		//�����������ô�죬�����ǲ���ҲӦ�����ݴ����????
			dataword = ptrace(PTRACE_PEEKDATA, pid, i, 0);
			fwrite(&dataword, 4, 1, outputfile);
		}
		//�������ļ�д�����
		//����ѡ�������λ�ÿ�ʼdump�������ļ���������о͵�ѡ�����ط�:)
		fclose(outputfile);
		
		//PTRACE_SYSCALL����ִ��һ��ָ���ֹͣ
		ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
		return 1;
	}
       //����ִ��һ��PTRACE_SYSCALL		
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
			//ִ�д���صĶ���
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
			//ͨ������0�������Ƿ����
			if(kill(mypid, 0) == -1)
			{
				fclose(fp);
				exit(0);
			}
			//PTRACE_SYSCALL����ÿ��ϵͳ���ý���/�˳�ʱֹͣ
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
					//���ȣ�ִ��ȥ��anti-ptrace�Ĵ���
					//Ȼ�󣬼�⵽read syscall������dump�ļ�
					state=handle_ptrace_syscall(mypid, fp);
				else
					//���������ǿ�ʼ����read���ú��ÿһ��ָ��
					state=handle_ptrace_singlestep(mypid, fp);
			}
	}
}

