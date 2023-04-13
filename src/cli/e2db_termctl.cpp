/*!
 * e2-sat-editor/src/cli/e2db_termctl.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "e2db_termctl.h"

namespace e2se_cli
{

#ifndef WIN32
bool tty_raw = false;
struct termios tty_attr;
#endif

//TODO
e2db_termctl::e2db_termctl()
{
	std::setlocale(LC_NUMERIC, "C");

	std::stringbuf* is_buf = new std::stringbuf;
	this->is = new std::iostream(is_buf);

	std::stringbuf* history_buf = new std::stringbuf;
	this->history = new std::iostream(history_buf);

	*history << "dolor amet lorem 5" << std::endl;
	*history << "lorem amet 4" << std::endl;
	*history << "ipsum dolor sit 3" << std::endl;
	*history << "sit dolor 2" << std::endl;
	*history << "amet lorem ipsum 1" << std::endl;

	this->last = this->history->tellg();
}

e2db_termctl::~e2db_termctl()
{
	reset();

	delete this->is;
}

void e2db_termctl::reset()
{
#ifndef WIN32
	tty_set_sane();
#endif
}

std::istream* e2db_termctl::input()
{
#ifndef WIN32
	tty_set_raw();
#endif

	std::printf("> ");

	size_t cur, len;
	cur = len = 0;

	int prev, next;
	prev = next = 0;

	std::string str;
	char c;
	while ((c = std::getchar()) != EOF)
	{
		// escape sequence
		if (c == 27)
		{
			while (! std::isalpha(c))
			{
				c = next = std::getchar();
			}
			switch (c)
			{
				// KeyUp
				// history previous
				case 65:
					{
						std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

						std::streampos pos = history->tellg();

						// if (pos == 0 && str.empty())
						// 	str = std::string (is_buf->str());

						// next repeat pos -1
						if (prev != next && pos == EOF)
						{
							history->clear();
							history->seekg(0);
							last = pos = 0;
						}
						// next repeat last 0
						else if (prev != 0 && last == 0)
						{
							history->clear();
							history->seekg(EOF);
							last = pos = EOF;
						}
						if (pos != EOF)
						{
							pos = 0;
							history->clear();
							history->seekg(pos);
							while (history->ignore(std::numeric_limits<std::streamsize>::max(), '\n'))
							{
								if (last)
									pos = history->tellg() != last ? history->tellg() : pos;
								else
									pos = history->tellg() != history->tellp() ? history->tellg() : pos;
								if (last == history->tellg())
									break;
							}
							history->clear();
							history->seekg(pos);
						}

						std::string line;

						if (std::getline(*history, line))
							last = pos;
						else
							tty_bell();

						if (! line.empty())
						{
							is_buf->str("");
							is->clear();

							std::printf("\r> ");
							for (int i = 0; i != len; i++)
								std::putchar(' ');

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
					}
					prev = c;
					continue;
				break;
				// KeyDown
				// history next
				case 66:
					{
						std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

						std::streampos pos = history->tellg();

						// next repeat pos -1
						if (prev != next && pos == EOF)
						{
							history->clear();
							history->seekg(0);
							last = pos = 0;
						}

						std::string line;

						if (std::getline(*history, line))
							last = pos;
						else if (str != is_buf->str())
							line = str;
						else
							tty_bell();

						// if (! line.empty())
						{
							is_buf->str("");
							is->clear();

							std::printf("\r> ");
							for (int i = 0; i != len; i++)
								std::putchar(' ');

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
					}
					prev = c;
					continue;
				break;
				// KeyRight
				// move cursor right
				case 67: // key Right
					if (cur < len && len != 0)
					{
						tty_gotoright();
						cur++;
					}
					else
					{
						tty_bell();
					}
					prev = c;
					continue;
				break;
				// KeyLeft
				// move cursor left
				case 68:
					if (cur > 0 && len != 0)
					{	
						tty_gotoleft();
						cur--;
					}
					else
					{
						tty_bell();
					}
					prev = c;
					continue;
				break;
			}
		}
		// KeyDel
		// delete char
		else if (c == 127)
		{
			if (cur > 0 && len != 0)
			{
				is->get(); // erase char
				tty_erase();
				cur--, len--;
			}
			else
			{
				tty_bell();
			}
			prev = c;
			continue;
		}
		// KeyReturn
		// stdin release
		else if (c == 10)
		{
			std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

			if (! is_buf->str().empty())
			{
				*history << std::endl;

				history->clear();
				history->seekg(0);
				last = history->tellg();
				str = "";
			}

			std::putchar(c);

			prev = c;
			break;
		}

		// std::printf("char: '%c'\n", c);

		//TODO FIX replace
		std::putchar(c);

		*is << c;
		*history << c;

		if (cur != len)
			cur++;
		else
			cur++, len++;
	}

#ifndef WIN32
	tty_set_sane();
#endif

	return is;
}

std::istream* e2db_termctl::clear()
{
	std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
	is->clear();
	is_buf->str("");

	return is;
}

void e2db_termctl::debugger()
{
	std::ofstream log ("./e2se-cli_log.txt");
	std::string line;

	log << "--- begin is      ---" << std::endl;
	is->clear();
	is->seekg(0);
	while (std::getline(*is, line, '\0'))
		log << line << std::endl;
	log << "--- end   is      ---" << std::endl;

	log << "--- begin history ---" << std::endl;
	history->clear();
	history->seekg(0);
	while (std::getline(*history, line))
		log << line << std::endl;
	log << "--- end   history ---" << std::endl;

	log << std::endl;

	log.close();
}

void e2db_termctl::tmp_history()
{
	std::ofstream log ("./e2se-cli_history");
	std::string line;

	history->clear();
	history->seekg(0);
	while (std::getline(*history, line))
		log << line << std::endl;

	log.close();
}

#ifndef WIN32
void e2db_termctl::tty_set_raw(int tty_fd)
{
	struct termios ta;
	tcgetattr(tty_fd, &ta);
	tty_attr = ta;
	tty_raw = true;
	ta.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(tty_fd, TCSANOW, &ta);
}

void e2db_termctl::tty_set_sane(int tty_fd)
{
	if (tty_raw)
		tcsetattr(tty_fd, TCSANOW, &tty_attr);
	tty_raw = false;
}
#endif

void e2db_termctl::tty_gotoxy(int x, int y)
{
#ifdef WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coordPos = {short (x), short (y)};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::printf("\e\x5b%d;%df", y, x);
#endif
}

void e2db_termctl::tty_gotoright()
{
#ifdef WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbi);
	COORD coordPos = {csbiInfo.dwCursorPosition.X + 1, csbiInfo.dwCursorPosition.Y};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::printf("\e\x5b\1\x43");
#endif
}

void e2db_termctl::tty_gotoleft()
{
#ifdef WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	COORD coordPos = {csbiInfo.dwCursorPosition.X - 1, csbiInfo.dwCursorPosition.Y};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::putchar('\b');
	// std::printf("\e\x5b\1\x44");
#endif
}

void e2db_termctl::tty_erase()
{
	std::printf("\b\e\x5b\x4b");
}

void e2db_termctl::tty_bell()
{
	std::putchar('\a');
}

}
