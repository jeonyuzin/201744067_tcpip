import socket
table={'1':'one','2':'two','3':'three','4':'four','5':'five'}

s=socket.socket()#AF_INET,SOCK_STEAM 기본값
address=("",2500)#localhost 기본 값
s.bind(address)
s.listen(1)
print('wating ...')
c_socket,c_addr=s.accept()
print('connection from',c_addr)

while True:
    data=c_socket.recv(1024).decode() #요청 수신
    try:
        resp=table[data]#데이터를 key 사용하여 value를 가져옴
    except:
        c_socket.send('Try againg'.encode())#오류가 있을때
    else:
        c_socket.send(resp.encode())#변환 값을 전송
