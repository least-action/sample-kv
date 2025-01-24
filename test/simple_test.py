import unittest
import socket

class GetSetDelTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._set_up_error = False
        try:
            cls._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # todo: port has to be from arg
            cls._socket.connect(("localhost", 1234))
            cls._socket.settimeout(1)
        except OSError as e:
            cls._set_up_error = True

    def setUp(self):
        if self._set_up_error:
            self.fail('set up error')

    def send_and_recv(self, binary_data):
        data = b''
        while not data.endswith(b'kv> '):
            chunk = self._socket.recv(1024)
            data += chunk

        self._socket.sendall(binary_data)
        data = b''
        while not data.endswith(b'\r\n\x00'):
            chunk = self._socket.recv(1024)
            data += chunk
        return data

    def test_get(self):
        data = self.send_and_recv(b'get 1\r\n')
        self.assertEqual(b'(nil)\r\n\x00', data)

    def test_set(self):
        data = self.send_and_recv(b'set 2 a\r\n')
        self.assertEqual(b'OK\r\n\x00', data)

        data = self.send_and_recv(b'get 2\r\n')
        self.assertEqual(b'a\r\n\x00', data)

    def test_del(self):
        data = self.send_and_recv(b'del 3\r\n')
        self.assertEqual(b'0\r\n\x00', data)

        data = self.send_and_recv(b'set 3 b\r\n')
        self.assertEqual(b'OK\r\n\x00', data)

        data = self.send_and_recv(b'get 3\r\n')
        self.assertEqual(b'b\r\n\x00', data)

        data = self.send_and_recv(b'del 3\r\n')
        self.assertEqual(b'1\r\n\x00', data)

        data = self.send_and_recv(b'get 3\r\n')
        self.assertEqual(b'(nil)\r\n\x00', data)

    @classmethod
    def tearDownClass(cls):
        cls._socket.close()

