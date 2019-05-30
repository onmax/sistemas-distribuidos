import sys
import re
import socket
import os

# Buffer de recepcion
MAX_BUFFER = 1024

foldername = "output"

# Lista con todos los sockets de los servidores a los que nos hemos podido conectar
sockets = []

# Lista que guarda las rutas relativas de los archivos
resources = []

# Parseo los argumentos de entrada
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

# Envio de peticiones de forma proporcional
for i, resource in enumerate(resources):
    send_str = "GET /%s HTTP/1.0\r\n\r\n" % resource
    sockets[i % len(sockets)].send(send_str.encode('utf-8'))

if not os.path.exists(foldername):
    os.makedirs(foldername)

print("Descargado %d archivo(s), guardando en:\n  /%s" %
      (len(resources), foldername))

# Recv de todos los sockets
for i, resource in enumerate(resources):
    response = b''
    while True:
        chunk = sockets[i % len(sockets)].recv(MAX_BUFFER)
        if len(chunk) <= 0:
            break
        response += chunk

    # Escribimos los resultados que estan en bytes en los archivos
    filename = resource.split("/")[-1]
    body = response.split(bytes('\r\n\r\n', "utf-8"))[1]
    with open("%s/%s" % (foldername, filename), "wb") as fd:
        fd.write(body)
    print("    |-- %s - (%d B)" % (filename, len(body)))

[s.close for s in sockets]
