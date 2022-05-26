import socket
sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

#서버 주소 입력
svrIP=input(("Server IP(default: 127.0.0.1): "))
if svrIP=='':
    svrIP='127.0.0.1' #기본 주소

#포트 번호 입력
port=input('port(default: 2500): ')
if port=='':
    port=2500#기본 포트
else:
    port= int(port)

sock.connect((svrIP,port))
print('Connected to ' + svrIP)
while True:
    msg=input("sending message: ")

    #송신 데이터가 없으면 다시 진행
    if not msg:
        continue
    try:#데이터 전송
        sock.send(msg.encode())#메시지 전송

    except:#연결이 종료됨
        print("연결이 종료되었습니다.")
        break
    try:#데이터 읽기
        msg=sock.recv(1024)
        if not msg:
            print("연결이 종료되었습니다.")
            break
        print(f'Recevied message : {msg.decode()}')
    except:#연결이 종료됨
        print("연결이 종료되었습니다.")
        break
sock.close()