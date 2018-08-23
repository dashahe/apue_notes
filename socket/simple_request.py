import socket

class responce:
    def __init__(self, header, html):
        self.header = header
        self.html = html

class simple_requests:
    def get(self, url):
        self.url = url
        self.port = 80

        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.url, self.port))
        self.s.send('GET / HTTP/1.1\r\nHost: {}\r\nConnection: close\r\n\r\n'.format(url[:url.find('/')]))

        buffer = []
        while True:
            d = self.s.recv(1024)
            if d:
                buffer.append(d)
            else:
                break
        data = ''.join(buffer)
        self.s.close()
        header, html = data.split('\r\n\r\n', 1)
        return responce(header, html)