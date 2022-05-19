#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char*argv[])
{
				int status;
				pid_t pid=fork(); //첫번째 자식 
				if(pid==0)
				{
								return 3; //첫번재 자식의 반환값
				}
				else
				{
								printf("child PID:%d \n",pid);
								pid=fork();//두 번재 자식 생성
								if(pid==0)
								{
												exit(7);
								}
								else
								{
												printf("Child PID : %d \n",pid);//두 번째 자식의 반환값
												wait(&status);
												if(WIFEXITED(status))
																printf("Child send one: %d \n", WEXITSTATUS(status));
												wait(&status);
												if(WIFEXITED(status))
																printf("Child send two: %d \n", WEXITSTATUS(status));
												printf("그냥 실행");
												sleep(30); //sleep 30 sec.
								}
				}
				return 0;
}

