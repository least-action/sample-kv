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
            data = b''
            while not data.endswith(b'connected\r\n'):
                chunk = cls._socket.recv(1024)
                data += chunk
        except OSError as e:
            cls._set_up_error_msg = e.msg
            cls._set_up_error = True

    def setUp(self):
        if self._set_up_error:
            self.fail('set up error: {}'.format(self._set_up_error_msg))

    def send_and_recv(self, binary_data):
        self._socket.sendall(binary_data)

        data = b''
        while not data.endswith(b'\r\n\x00'):
            chunk = self._socket.recv(1024)
            data += chunk
        return data

    def test_get(self):
        print("test_get start")
        data = self.send_and_recv(b'get 1\r\n')
        self.assertEqual(b'(nil)\r\n\x00', data)

    def test_set(self):
        print("test_set start")
        data = self.send_and_recv(b'set 2 a\r\n')
        self.assertEqual(b'OK\r\n\x00', data)

        data = self.send_and_recv(b'get 2\r\n')
        self.assertEqual(b'a\r\n\x00', data)

    def test_del(self):
        print("test_del start")
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

