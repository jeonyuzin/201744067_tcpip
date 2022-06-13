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
