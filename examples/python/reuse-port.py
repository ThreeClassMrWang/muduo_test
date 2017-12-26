#!/usr/bin/env python

# Sample code for the example of REUSE_PORT option for linux who's kernel larger than 3.9
# Just run the server, it listen on port 1234. Connect to it .
# Now run another server process, and a few more, you'll see the kernel allows these all to listen to
# the same port. The kernel will picks one of the processes and allows it to handle it. Next time it
# maybe another process, you can add or remove processes.

from os import getpid
from socket import *

port = 1234

server = socket(AF_INET, SOCK_STREAM)
server.setsockopt(SOL_SOCKET, SO_REUSEPORT, 1)
server.bind(('', port));
server.listen(10)

print('[python pid:{}] listening on port {}...'.format(getpid(), port))

while True:
    (client, clientaddr) = server.accept()
    print('[python pid:{}] got connection from {}'.format((), client.getpeername()))
    client.send(bytes('Hello from python process {}\n'.format(getpid()), 'utf-8'))
    client.close()

