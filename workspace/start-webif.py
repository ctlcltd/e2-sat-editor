#!/usr/bin/env python3
# A virtual WebIf

from http.server import SimpleHTTPRequestHandler
from socketserver import TCPServer
from time import sleep

class htsh(SimpleHTTPRequestHandler):
	def do_GET(self):
		if self.path.startswith("/web/servicelistreload"):
			sleep(5)
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

httpd = TCPServer(("127.0.0.1", 8080), htsh)
print("Virtual WebIf started at port 8080")
httpd.serve_forever()
