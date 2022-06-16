#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>


#define BUF_SIZE 100
void error_handling(char *buf);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;//소켓 파일디스크립터변수
	struct sockaddr_in serv_adr,clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;
	socklen_t adr_sz;
	int fd_max, str_len,fd_num,i;
	char buf[BUF_SIZE];
	serv_sock=socket(PF_INET,SOCK_STREAM,0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
		error_handling("listen() error");

	FD_ZERO(&reads);//주소의 fd_set형 변수의 모든 비트를 초기화
	//서버소켓(리스닝소켓)을 통한 연결요청도 일종의
			//데이터 수신이기때문에 관찰의 대상에 포함을 시킨다.
	FD_SET(serv_sock,&reads);//서버소켓으로 전달된 파일디스크립터정보등록
	fd_max=serv_sock;
	while(1)
	{
		cpy_reads=reads;//복사
		timeout.tv_sec=5;
		timeout.tv_usec=5000;
		if((fd_num=select(fd_max+1,&cpy_reads,0,0,&timeout))==-1)
			break;
		if(fd_num==0)
			continue;

	}

	for(i=0; i<fd_max+1; i++)
	{
		if(FD_ISSET(i,&cpy_reads))
		{
			if(i==serv_sock)//서버면(리스닝소켓)
			{
				adr_sz=sizeof(clnt_adr);
				clnt_sock=
					accept(serv_sock,(struct sockaddr*)&clnt_adr,&adr_sz);
				FD_SET(clnt_sock,&reads);//reads에 클라이언트소켓등록
				if(fd_max<clnt_sock)
					fd_max=clnt_sock;//최대검사할파일디스크립터수를변경
				printf("conneted client: %d \n",clnt_sock);

			}
		}
		else //read message 클라이어ㄴ트임
		{
			str_len=read(i,buf,BUF_SIZE);
			if(str_len==0)//close reqeuset
			{
				FD_CLR(i,&reads);//매개변수 fdsetㅇ으로 전달된 주소의 변수에서 매개변수fd로전달된 정보를 삭제
				close(i);//삭제하고 소켓 종료
				printf("closed clinet: %d \n",i);
			}
			else
			{
				write(i,buf,str_len);//echo!
			}
		}
	}
}
void error_handling(char *message)
{
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
