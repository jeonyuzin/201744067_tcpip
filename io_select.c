#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 30


int main(int argc,char *argv[])
{
	fd_set reads,temps;
	int result, str_len;
	char buf[BUF_SIZE];
	struct timeval timeout;
	FD_ZERO(&reads);
	FD_SET(0,&reads); //0 is standard input(console)
	//timeout.tv_sec=5;
	//timeout.tv_usec=5000;
	//이거 잘못된거 아래에 설명
	
	while(1)
	{
		temps=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		result=select(1,&temps,0,0,&timeout);
		//timeout을 여기서 초기화해야하는 이유
		//select시 5초라고 설정하고 3초남았을때 select수행함
		//그럼 timeout은 3초를 기준으로 됨.
		//따라서 select직전에 while루틴안에서 초기화해야함
		//그냥 무조건 초괴화한다 생각.
		if(result==-1)//데이터가없으면
		{
			puts("select() error!");
			break;
		}
		else if(result==0)
		{
			puts("Time-out!");
		}
		else
		{
			if(FD_ISSET(0,&temps))//파일디스크립터0의변화관찰
			{
				str_len=read(0,buf,BUF_SIZE);
				buf[str_len]=0;
				printf("message from console: %s",buf);
			}
		}
	}
	return 0;
			  
}

