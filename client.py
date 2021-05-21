import socket
import sys

server_ip = sys.argv[1]

def msg_len(msg):
    send_len = str(len(msg)).encode('utf-8')
    send_len+= b' ' * (64-len(send_len))
    return send_len

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("Enter #end# to end connection")
while True:
    format = 'utf-8'
    num = input("Enter number: ")
    num = num.encode(format)
    client.sendto(msg_len(num),(server_ip,5000))
    client.sendto(num,(server_ip,5000))
    if(num.decode(format) == '#end#'):
        break
    msg = client.recvfrom(10240)
    print("Factorial is "+msg[0].decode(format))







