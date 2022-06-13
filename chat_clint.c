#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>



#define BUF_SIZE 100//메시지 버퍼의 길이
#define NAME_SIZE 20//유저 구분용 닉네임 버퍼의길이


//함수 프로토타입 선언
void *send_msg(void *arg); 
void *recv_msg(void *arg);
void error_handling(char *msg);


//변수 선언
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc,char *argv[])
{
	int sock; //파일 디스크립터 변수
	struct sockaddr_in serv_addr;//서버 소켓의 구조체 변수
	pthread_t snd_thread, rcv_thread;//snd쓰레드, rcv쓰레드
	void *thread_return;//쓰레드 반환 값


	//클라이언트이므로 IP PORT NAME필요 name채팅창에서 유저구분
	if(argc!=4){ 
		printf("Usage : %s <IP> <port> <name>\n",argv[0]);
		exit(1);
	}

	sprintf(name,"[%s]",argv[3]);//사용자의 이름 출력
	sock=socket(PF_INET,SOCK_STREAM,0);//IPV4로 TCP소켓통신

	memset(&serv_addr,0,sizeof(serv_addr));//서버 소켓의 정보를 담을 구조체를 0으로 초기화한다.
	serv_addr.sin_family=AF_INET;//IPV4를 사용한다.
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);//입력받은 IP를 16진수로 변경해서 저장
	serv_addr.sin_port=htons(atoi(argv[2]));//입력된 포트번호를 10진수로 변경해서 저장


	//TCP/IP통신 방식 중 클라이언트에서 connect
	//connect실패 시 -1리턴하므로 if문을 통해 error_handling 호출
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
		error_handling("connect() error");

	//send,POSIX쓰레드 생성
	//snd_thread에 쓰레드의 id를 저장 ,속성 없음
	//쓰레드에 의해 호출되는 send_msg함수 , 함수에 전달할 인자 값 (void*)&sock
	pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);

	//rcv_thread에 쓰레드의 id를 저장 ,속성 없음
	//쓰레드에 의해 호출되는 recv_msg함수 , 함수에 전달할 인자 값 (void*)&sock
	pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);

	//pthread_join함수는 인자로 전달되는 ID에 해당하는 쓰레드가 종료될때까지 대기 상태에 들어감.
	//따라서 메인함수는 쓰레드가 종료될때까지 기다리고 2개의 쓰레드로 통신한다.
	pthread_join(snd_thread,&thread_return);
	pthread_join(rcv_thread,&thread_return);
	close(sock);//종료시 소켓 소멸
	return 0;
}

void *send_msg(void *arg)
{
	int sock=*((int*)arg); //pthread_create에 의해 소켓을 전달받는다.
	char name_msg[NAME_SIZE+BUF_SIZE]; //둘이 합쳐서 받음
	while(1)//무한 루프
	{
		fgets(msg,BUF_SIZE,stdin); //키보드의 입력을 받고
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) //q나Q면 소켓을 종료하고 메인도종료
		{
			close(sock);
			exit(0);
		}
		sprintf(name_msg,"%s %s",name,msg);//그외 채팅을 입력시 이름+띄어쓰기+메시지를 저장 후
		write(sock,name_msg,strlen(name_msg)); 길이와함께 서버로 보낸다.
	}
	return NULL;
}

//RECVIE용 쓰레드 함수
void *recv_msg(void *arg)
{
	int sock=*((int*)arg);//소켓을 전달받는다.
	char name_msg[NAME_SIZE+BUF_SIZE];//이름과 메시지의 길이를 합친 문자열
	int str_len;//길이를 위한 int변수
	while(1)무한루프
	{
		str_len=read(sock,name_msg,NAME_SIZE+BUF_SIZE-1);//메시지를 read
		if(str_len==-1)//read가 -1이면  통신이 끊겼으므로 
			return (void*)-1; 쓰레드 종료를 위한 return값을 전달
		name_msg[str_len]=0;//메시지길의 마지막을 0으로
		fputs(name_msg,stdout);//클라이언트에 받은 메시지 출력

	}
	return NULL;
}
void error_handling(char *msg)
{
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}
