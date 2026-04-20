#!/usr/bin/env python3
# A virtual FTP server

HOST_USER = "root"
HOST_PWRD = "password"
HOST_ADDR = "127.0.0.1"
HOST_PORT = 2121

import sys
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler, ThrottledDTPHandler
from pyftpdlib.servers import FTPServer

FILE_PATH = sys.argv[1]
HOST_THROTTLE_SPEED = int(sys.argv[2]) if len(sys.argv) == 3 else 0

#testing internal error for 'str' expected 'int'
# HOST_THROTTLE_SPEED = sys.argv[2]

authorizer = DummyAuthorizer()
authorizer.add_user(HOST_USER, HOST_PWRD, FILE_PATH, perm="elradfmwMT")

ftp_handler = FTPHandler
ftp_handler.authorizer = authorizer

if HOST_THROTTLE_SPEED:
	print("[throttle speed", HOST_THROTTLE_SPEED, "Kb/s]")
	dtp_handler = ThrottledDTPHandler
	dtp_handler.read_limit = HOST_THROTTLE_SPEED * 1024
	dtp_handler.write_limit = HOST_THROTTLE_SPEED * 1024
	ftp_handler.dtp_handler = dtp_handler

server = FTPServer((HOST_ADDR, HOST_PORT), ftp_handler)
server.serve_forever()
