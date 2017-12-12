#!/usr/bin/python3

import socket


def handle(client_socket, client_address):
    while True:
        data = client_socket.recv(4096)
        if data:
            sent = client_socket.sendall(data)     # send all?
        else:
            print("disconnect", client_address)
            client_socket.close()
            break


if __name__ == "__main__":
    # main function for test iterative echo server
    # server listen on port in localhost
    # wait for connection.
    # It can only handle on connection on time.
    listen_address = ("0.0.0.0", 2007)
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(listen_address)
    server_socket.listen(5)

    while True:
        (client_socket, client_address) = server_socket.accept()
        print("got connection from", client_address)
        handle(client_socket, client_address)
