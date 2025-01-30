import unittest
import socket

class SeparatedPacketTest(unittest.TestCase):
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

        self.send(b'get abcd\r')
        self.send(b'\n')
        data = self.receive()
        self.assertEqual(b'wxyz\r\n\x00', data)

    def test_multi_command_with_one_packet(self):
        print("test_two_command_with_one_packet start")
        self.send(b'get abcde\r\nset abcde wxyz\r\nget abcde\r\n')
        data = b''
        for i in range(3):
            data += self.receive()
            if data.count(b'\r\n\x00') == 3:
                break
        self.assertEqual(b'(nil)\r\n\x00OK\r\n\x00wxyz\r\n\x00', data)

    def test_separated_packet_2(self):
        print("test_separated_packet_2 start")
        self.send(b'get abcdef\r\nset abc')
        data = self.receive()
        self.assertEqual(b'(nil)\r\n\x00', data)
        self.send(b'def wxyz\r\nget abcdef\r\n')
        data = b''
        for i in range(2):
            data += self.receive()
            if data.count(b'\r\n\x00') == 2:
                break
        self.assertEqual(b'OK\r\n\x00wxyz\r\n\x00', data)

    def tearDown(self):
        self._socket.close()

