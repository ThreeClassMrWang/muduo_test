#!/usr/bin/python3

# This is echo reactor test
# We deal based events and react events to their handlers

import socket
import select

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(("0.0.0.0", 2007))
server_socket.listen(5)

poll = select.poll()
connections = {}
handlers = {}


def handle_input(socket, data):
    print("handle input from fileno ", fileno)
    socket.send(data)


def handle_request(fileno, event):
    print("handle request from fileno ", fileno)
    if event & select.POLLIN:
        client_socket = connections[fileno]
        data = client_socket.recv(4096)
        if data:
            handle_input(client_socket, data)
        else:
            poll.unregister(fileno)
            client_socket.close()
            del connections[fileno]
            del handlers[fileno]


def handle_accept(fileno, event):
    print("handle accept from fileno ", fileno)
    (client_socket, client_address) = server_socket.accept()
    print("got connenction from ", client_address)
    poll.register(client_socket.fileno(), select.POLLIN)
    connections[client_socket.fileno()] = client_socket
    handlers[client_socket.fileno()] = handle_request


poll.register(server_socket.fileno(), select.POLLIN)
handlers[server_socket.fileno()] = handle_accept

while True:
    events = poll.poll(10000)
    for fileno, event in events:
        handler = handlers[fileno]
        handler(fileno, event)
