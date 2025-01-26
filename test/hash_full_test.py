import unittest
import socket

class HashFullTest(unittest.TestCase):
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

    def test_set_many(self):
        print("test_set_many start")
        is_failed = False
        failed_i = 0
        data = None
        test_id = hash(self)
        for i in range(100):
            data = self.send_and_recv('set {}{} 1\r\n'.format(test_id, i).encode())
            if b'OK\r\n\x00' != data:
                is_failed = True
                failed_i = i
                break
        for i in range(failed_i):
            self.send_and_recv('del {}{}\r\n'.format(test_id, i).encode())

        if is_failed:
            self.fail("failed on {} with {}".format(i, data))


    @classmethod
    def tearDownClass(cls):
        cls._socket.close()

