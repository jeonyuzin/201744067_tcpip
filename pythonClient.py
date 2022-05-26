import socket
port=2500
address=("localhost",port)
BUFSIZE=1024


s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.connect(address)
while True:
    msg=input("Message to send:")
    s.send(msg.encode())#send a message to server
    r_msg=s.recv(BUFSIZE)#receive message from server
    if not r_msg:break
    print("Recevied message: %s" %r_msg.decode())

