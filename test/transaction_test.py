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

    def test_no_dirty_read(self):
        print("test_no_dirty_read")
        test_id = abs(hash(self))
        socket1 = self.create_connection()
        socket2 = self.create_connection()

        # Arrange
        self.send(socket1, 'begin\r\n'.encode())
        self.receive(socket1)
        self.send(socket1, 'set {} tt\r\n'.format(test_id).encode())
        self.receive(socket1)

        # Act
        self.send(socket1, 'get {}\r\n'.format(test_id).encode())
        self.send(socket2, 'get {}\r\n'.format(test_id).encode())
        data1 = self.receive(socket1)
        data2 = self.receive(socket2)

        # Assert
        self.assertEqual(b'tt\r\n\x00', data1)
        self.assertEqual(b'(nil)\r\n\x00', data2)

        self.send(socket1, 'del {}\r\n'.format(test_id).encode())
        self.receive(socket1)
        socket2.close()
        socket1.close()

    def test_conflict(self):
        print("test_conflict")
        test_id = abs(hash(self))
        socket1 = self.create_connection()
        socket2 = self.create_connection()

        # Arrange
        self.send(socket1, 'begin\r\n'.encode())
        self.receive(socket1)
        self.send(socket1, 'get {}\r\n'.format(test_id).encode())
        self.receive(socket1)
        self.send(socket2, 'set {} aa\r\n'.format(test_id).encode())
        self.receive(socket2)


        # Act
        self.send(socket1, 'get {}\r\n'.format(test_id).encode())
        self.send(socket2, 'get {}\r\n'.format(test_id).encode())
        data1 = self.receive(socket1)
        data2 = self.receive(socket2)

        # Assert
        self.assertEqual(b'conflict\r\n\x00', data1)
        self.assertEqual(b'aa\r\n\x00', data2)

        self.send(socket2, 'del {}\r\n'.format(test_id).encode())
        self.receive(socket2)
        socket2.close()
        socket1.close()

