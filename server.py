import socket

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('', 5000))

def factorial(num):
    res = 1
    for i in range(1, num+1):
        res*=i
    return res

print("Server started. Press Ctrl + C to exit")
while True:
    format = 'utf-8'
    msg_len, client_addr = server.recvfrom(64)
    msg_len = msg_len.decode(format)
    if msg_len:
        msg_len = int(msg_len)
        msg, client_addr = server.recvfrom(msg_len)
        msg = msg.decode(format)
        if(msg == '#end#'):
            break
        try:
            res = factorial(int(msg))
        except ValueError:
            res = "not a valid number.\nPlease send a number or #end# to end connection"
        print("Output is " + str(res))
        res = str(res)
        try:
            server.sendto(res.encode(format), client_addr)
        except OSError:
            server.sendto("too long to print".encode(format), client_addr)