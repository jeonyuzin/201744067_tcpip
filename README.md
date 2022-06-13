# 201744067_tcpip
20174406전유진 기말고사 대체 과제물 코드 해석 (README용 별도의 파일도 깃허브에 존재)
==============================================================================
=============================================================================
serv단
==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


#define BUF_SIZE 100//버퍼사이즈
#define MAX_CLNT 2//최대 클라이언트수

//함수 프로토타입 선언
void * handle_clnt(void * arg);
void send_msg(char * msg,int len);
void error_handling(char * msg);


//변수 초기화
int clnt_cnt=256;
int clnt_socks[MAX_CLNT];//서버에 접속한 클라이언트의 소켓관리를 위한 변수
pthread_mutex_t mutx;//뮤텍스 변수 선언


int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;//서버와 클라이언트 소켓생성할 변수 
	struct sockaddr_in serv_adr,clnt_adr; //서버와 클라이언트 소켓의 구조체 
	int clnt_adr_sz;//클라이언트 소켓 정보를 저장할 구조체의 길이를 저장할 변수
	pthread_t t_id;//POSIX 쓰레드를 생성할 변수
	if(argc!=2){//서버이므로 포트만 입력 INDDR_ANY로 자동으로 IP주소가져온다.
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx,NULL);//뮤텍스 참조 값 저장을 위한 변수의 주소 값 전달,별도의 특성을 지정하지 않으므로 NULL
	serv_sock=socket(PF_INET,SOCK_STREAM,0);//서버 소켓 생성 IPV4,TCP방식을 사용한다.

	
	memset(&serv_adr,0,sizeof(serv_adr));//서버 소켓의 정보를 담을 구조체를 0으로 초기화한다.
	serv_adr.sin_family=AF_INET;//IPV4를 사용한다.
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);//자기 자신의 아이피를  16진수로 변경해서 저장
	//host to number long
	serv_adr.sin_port=htons(atoi(argv[1]));//입력된 포트번호를 10진수로 변경해서 저장 
	//ascii to number short


	//TCP/IP통신 중 서버의 통신 절차 중 서버소켓과 구조체의 정보를 이용해서 bind와 listen을하고 에러시 -1을 반환하므로
	//if문을 통해 에러 메시지 출력
	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
		error_handling("listen() error");


	//listen이후 accpet할 차례	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);//클라이언 구조체의 길이에 할당
		clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz)//;//클라이언트의 접속을 받아들임

		pthread_mutex_lock(&mutx);//뮤텍스로 lock 등록중 문제생기면안되므로
		clnt_socks[clnt_cnt++]=clnt_sock; //클라이언트 수와 파일 디스크립터 등록
		pthread_mutex_unlock(&mutx); //뮤텍스unlock

		pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt_sock);//POSIX쓰레드 생성
		//t_id에 쓰레드의id를 저장, 속성 없음NULL, handle_clnt 실행할 함수 루틴, 
		//(void*)&clnt_sock 쓰레드에 의해 호출되는 함수에 전달할 인자값
		pthread_detach(t_id);//쓰레드가 종료되면 스스로 소멸시킨다.
		printf("Connected client IP: %s =n",inet_ntoa(clnt_adr.sin_addr));

	}
	close(serv_sock);//쓰레드가 끝나면 소켓연결을 종료한다.
	return 0;
}


//쓰레드에서 호출하는 함수
void * handle_clnt(void *arg)
{
	int clnt_sock=*((int*)arg);.//통신할 클라이언트 소켓 변수를 받음
	
	//메시지 버퍼와 파일디스크립터 변수,
	int str_len=0, i;
	char msg[BUF_SIZE];

	//read함수를 반복으로 호출해 eof까지 읽음
	//read(accept로 생성된 소켓  번호, 읽어드린 버퍼 변수, 읽어드린 데이터의크기)	
	while((str_len=read(clnt_sock,msg,sizeof(msg)))!=0)
		send_msg(msg,str_len); //통신가능하면 send_msg실행

	//disconnected된 클라이언트를 덮어씌워 삭제하므로 뮤텍스 잠금
	pthread_mutex_lock(&mutx); 
	for(i=0; i<clnt_cnt; i++)//클라이언트가 연결됬을때마다 증가했으므로 연결 수 만큼 반복
	{
		if(clnt_sock==clnt_socks[i])//현재 해당하는 파일 디스크립터를 찾으면
		{
			while(i++<clnt_cnt-1)//해당 정보를 덮어씌워 삭제
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;클라이언트 수 감소
	pthread_mutex_unlock(&mutx);//뮤텍스 해제
	close(clnt_sock);//연결 소켓 종료
	return NULL;
}

void send_msg(char *msg, int len)
{
	int i;
	pthread_mutex_lock(&mutx);임계영역
	for(i=0; i<clnt_cnt; i++)//전체 클라이언트수만큼
		write(clnt_socks[i],msg,len); //write를 한다. 즉 모든 클라이언트에게 메시지 전송
	pthread_mutex_unlock(&mutx);

}

void error_handling(char *msg)
{
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}
=========================================================================
=========================================================================
                          클라이언트단
========================================================================
                          #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//유닉스 표준
#include <string.h>
#include <arpa/inet.h>//인터넷 프로토콜
#include <sys/socket.h>//소켓
#include <pthread.h>//posxi쓰레드



#define BUF_SIZE 100//메시지 버퍼의 길이
#define NAME_SIZE 20//유저 구분용 닉네임 버퍼의길이


//함수 프로토타입 선언
void *send_msg(void *arg); 
void *recv_msg(void *arg);
void error_handling(char *msg);


//송신할 메시지와 이름 변수 선언
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
