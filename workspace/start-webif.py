#!/usr/bin/env python3
# A virtual WebIf

HOST_ADDR = "127.0.0.1"
HOST_PORT = 8080

import sys
from time import sleep
from http.server import SimpleHTTPRequestHandler
from socketserver import TCPServer

HOST_RESPONSE_DELAY = int(sys.argv[1]) if len(sys.argv) == 2 else 5

class Webif(SimpleHTTPRequestHandler):
	def do_GET(self):
		if self.path.startswith("/web/servicelistreload"):
			sleep(HOST_RESPONSE_DELAY)
			self.send_response(200)
			self.send_header("Content-Type", "text/xml")
			self.end_headers()
			self.wfile.write(b'<?xml version="1.0"?>\n\
<e2simplexmlresult><e2state>True</e2state><e2statetext>reloaded both</e2statetext></e2simplexmlresult>\n')
			return

		sleep(1)
		self.send_response(404)
		self.send_header("Content-Type", "text/html")
		self.end_headers()
		self.wfile.write(b'<HTML><HEAD><TITLE>404 Error 404</TITLE></HEAD>\n\
<BODY><H4>404 Error 404</H4>\n\
File not found.\n\
</BODY></HTML>\n')

httpd = TCPServer((HOST_ADDR, HOST_PORT), Webif)
print("Virtual WebIf started at port", HOST_PORT)
httpd.serve_forever()
