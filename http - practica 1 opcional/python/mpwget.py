import sys
import re
import socket

sockets = []
resources = []

# Parser
for item in sys.argv[1:]:
    if item[0] == ":":
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((item[1:], 80))
            sockets.append(s)
        except socket.error as exc:
            print("Caught exception socket.error : %s %s" % (exc, item[1:]))
    else:
        resources.append(item)

for i,resource in enumerate(resources):
    send_str = "GET /%s HTTP/1.0\r\n\r\n" % resource
    sockets[i % len(sockets)].sendall(send_str.encode('utf-8'))

for i,resource in enumerate(resources):
    response = b''
    while True:
        chunk = sockets[i % len(sockets)].recv(1024)
        if len(chunk) <= 0:
            break
        response += chunk
    
    with open(resource.split("/")[-1], "wb") as fd:
        fd.write(response.decode().split('\r\n\r\n')[1].encode())