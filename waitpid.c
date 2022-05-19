#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork();

	if(pid==0)
	{
		sleep(15);
		return 24;
	}
	else
	{
		while(!waitpid(pid,&status,WNOHANG))//pid자리에 -1넣으면 임의의 자식프로세스
		{
			sleep(3);
			puts("sleep 3sec.");
		}
		if(WIFEXITED(status))
			printf("Child send %d \n",WEXITSTATUS(status));
	}
	return 0;
}

