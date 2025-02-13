import unittest
import socket

class HashFullTest(unittest.TestCase):
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

    def test_set_many(self):
        print("test_set_many start")
        is_failed = False
        data = None
        test_id = abs(hash(self))
        for i in range(100):
            self.send('set {}{} {}{}\r\n'.format(test_id, i, test_id, i).encode())
            data = self.receive()
            if b'OK\r\n\x00' != data:
                is_failed = True
                break

        for i in range(100):
            self.send('get {}{}\r\n'.format(test_id, i).encode())
            data = self.receive()
            if '{}{}\r\n\x00'.format(test_id, i).encode() != data:
                is_failed = True
                break

        for i in range(100):
            self.send('del {}{}\r\n'.format(test_id, i).encode())
            data = self.receive()
            if b'1\r\n\x00' != data:
                is_failed = True
                break

        if is_failed:
            self.fail("failed on {} with {}".format(i, data))

    def tearDown(self):
        self._socket.close()

