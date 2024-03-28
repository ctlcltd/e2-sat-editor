#!/usr/bin/env python3
# A virtual Telnet server

import asyncio
import telnetlib3

CR, LF, NUL = "\r\n\x00"

async def test_shell(reader, writer):
	attempts = 3

	while True:
		auth = await login(reader, writer)

		if auth is True:
			break
		else:
			attempts -= 1

		if attempts == 0:
			writer.close()

	writer.write("done!" + CR + LF)
	linereader = readline(reader, writer)
	linereader.send(None)

	command = None
	while True:
		if command:
			writer.write(CR + LF)
		writer.write("[root@STB /]# ")
		command = None
		while command is None:
			inp = await reader.read(1)
			if not inp:
				return
			command = linereader.send(inp)
		writer.write(CR + LF)
		if command == "exit":
			break
		elif command == "reboot":
			break
		elif command.startswith("init") or command.startswith("sleep"):
			break
		elif command:
			writer.write("-sh: " + command + ": not found")
	writer.close()

async def login(reader, writer):
	username = None
	password = None

	writer.write("STB login: ")
	username_reader = readline(reader, writer)
	username_reader.send(None)
	while username is None:
		inp = await reader.read(1)
		if not inp:
			return
		username = username_reader.send(inp)
	writer.write(CR + LF)

	if username == "root":
		writer.write("Password: ")
		password_reader = readline(reader, writer, True)
		password_reader.send(None)
		while password is None:
			inp = await reader.read(1)
			if not inp:
				return
			password = password_reader.send(inp)
		writer.write(CR + LF)

		if password == "root":
			return True
		else:
			writer.write("Login incorrect" + CR + LF)
			return False
	else:
		writer.write("Login incorrect" + CR + LF)
		return False

def readline(reader, writer, hide_input=False):
	command, inp, last_inp = '', '', ''
	inp = yield None
	while True:
		if inp in (LF, NUL) and last_inp == CR:
			last_inp = inp
			inp = yield None

		elif inp in (CR, LF):
			last_inp = inp
			inp = yield command
			command = ''

		elif inp in ('\b', '\x7f'):
			if command:
				command = command[:-1]
				writer.echo('\b \b')
			last_inp = inp
			inp = yield None

		else:
			command += inp
			ep = inp if not hide_input else ''
			writer.echo(ep)
			last_inp = inp
			inp = yield None

async def main():
	server = await telnetlib3.create_server(port=23, shell=test_shell)
	print("Virtual Telnet started at port 23")
	await server.wait_closed()

asyncio.run(main())
