import socket
import fcntl
import os, os.path
import time
import warnings

from tornado.escape import json_encode

from config import server_socket_path

class WebtilesSocketConnection(object):
    def __init__(self, io_loop, socketpath):
        self.io_loop = io_loop
        self.crawl_socketpath = socketpath
        self.message_callback = None
        self.socket = None
        self.socketpath = None

        self.msg_buffer = None

    def connect(self, primary = True):
        if not os.path.exists(self.crawl_socketpath):
            # Wait until the socket exists
            self.io_loop.add_timeout(time.time() + 1, self.connect)
            return

        self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)

        # Set close-on-exec
        flags = fcntl.fcntl(self.socket.fileno(), fcntl.F_GETFD)
        fcntl.fcntl(self.socket.fileno(), flags | fcntl.FD_CLOEXEC)

        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # Bind to a temp path
        # Ignore the security warning about tempnam; in this case,
        # there is no security risk (the most that can happen is that
        # the bind call fails)
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            self.socketpath = os.tempnam(server_socket_path, "crawl")
        self.socket.bind(self.socketpath)

        # Install handler
        self.io_loop.add_handler(self.socket.fileno(),
                                 self._handle_read,
                                 self.io_loop.ERROR | self.io_loop.READ)

        msg = json_encode({
                "msg": "attach",
                "primary": primary
                })

        self.socket.sendto(msg, self.crawl_socketpath)

    def _handle_read(self, fd, events):
        if events & self.io_loop.READ:
            data = self.socket.recv(128 * 1024, socket.MSG_DONTWAIT)

            self._handle_data(data)

        if events & self.io_loop.ERROR:
            pass

    def _handle_data(self, data):
        if self.msg_buffer is not None:
            data = self.msg_buffer + data

        if data[-1] != "\n":
            # All messages from crawl end with \n.
            # If this one doesn't, it's fragmented.
            self.msg_buffer = data

        else:
            self.msg_buffer = None

            if self.message_callback:
                self.message_callback(data)

    def send_message(self, data):
        self.socket.sendto(data, self.crawl_socketpath)

    def close(self):
        if self.socket:
            self.io_loop.remove_handler(self.socket.fileno())
            self.socket.close()
            os.remove(self.socketpath)
            self.socket = None
