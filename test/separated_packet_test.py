import unittest
import socket

class SeparatedPacketTest(unittest.TestCase):
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
            cls._set_up_error = True

    def setUp(self):
        if self._set_up_error:
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

    def test_separated_packet(self):
        print("test_separated_packet start")
        self.send(b'get ab')
        self.send(b'cd\r\n')
        data = self.receive()
        self.assertEqual(b'(nil)\r\n\x00', data)

        self.send(b'set ab')
        self.send(b'cd wxyz\r\n')
        data = self.receive()
        self.assertEqual(b'OK\r\n\x00', data)

        self.send(b'get ab')
        self.send(b'cd\r\n')
        data = self.receive()
        self.assertEqual(b'wxyz\r\n\x00', data)

    @classmethod
    def tearDownClass(cls):
        cls._socket.close()

