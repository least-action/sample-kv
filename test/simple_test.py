import unittest
import socket

class GetSetDelTest(unittest.TestCase):
    def setUp(self):
        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # todo: port has to be from arg
            self._socket.connect(("localhost", 1234))
            self._socket.settimeout(1)
            data = b''
            while not data.endswith(b'connected\r\n'):
                chunk = self._socket.recv(1024)
                data += chunk
        except OSError as e:
            self.fail('set up error')

    def send(self, binary_data):
        self._socket.sendall(binary_data)

    def receive(self):
        try_count = 0
        data = b''
        while not data.endswith(b'\r\n\x00'):
            if try_count > 100:
                raise Exception ("excceded limit try count")
            chunk = self._socket.recv(1024)
            data += chunk
            try_count += 1
        return data

    def test_get(self):
        print("test_get start")
        self.send(b'get 1\r\n')
        data = self.receive()
        self.assertEqual(b'(nil)\r\n\x00', data)

    def test_set(self):
        print("test_set start")
        self.send(b'set 2 a\r\n')
        data = self.receive()
        self.assertEqual(b'OK\r\n\x00', data)

        data = self.send(b'get 2\r\n')
        data = self.receive()
        self.assertEqual(b'a\r\n\x00', data)

    def test_del(self):
        print("test_del start")
        data = self.send(b'del 3\r\n')
        data = self.receive()
        self.assertEqual(b'0\r\n\x00', data)

        data = self.send(b'set 3 b\r\n')
        data = self.receive()
        self.assertEqual(b'OK\r\n\x00', data)

        data = self.send(b'get 3\r\n')
        data = self.receive()
        self.assertEqual(b'b\r\n\x00', data)

        data = self.send(b'del 3\r\n')
        data = self.receive()
        self.assertEqual(b'1\r\n\x00', data)

        data = self.send(b'get 3\r\n')
        data = self.receive()
        self.assertEqual(b'(nil)\r\n\x00', data)

    def tearDown(self):
        self._socket.close()

