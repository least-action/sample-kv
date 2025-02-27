import unittest
import socket

class TransactionTest(unittest.TestCase):
    @staticmethod
    def create_connection():
        _socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # todo: port has to be from arg
        _socket.connect(("localhost", 1234))
        _socket.settimeout(1)
        data = b''
        while not data.endswith(b'connected\r\n'):
            chunk = _socket.recv(1024)
            data += chunk
        return _socket

    @staticmethod
    def send(_socket, binary_data):
        _socket.sendall(binary_data)

    @staticmethod
    def receive(_socket):
        try_count = 0
        data = b''
        while not data.endswith(b'\r\n\x00'):
            if try_count > 100:
                raise Exception ("excceded limit try count")
            chunk = _socket.recv(1024)
            data += chunk
            try_count += 1
        return data

