#!/usr/bin/env python3
# A virtual FTP server

import sys
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

authorizer = DummyAuthorizer()
authorizer.add_user("root", "password", sys.argv[1], perm="elradfmwMT")

handler = FTPHandler
handler.authorizer = authorizer

server = FTPServer(("127.0.0.1", 2121), handler)
server.serve_forever()
