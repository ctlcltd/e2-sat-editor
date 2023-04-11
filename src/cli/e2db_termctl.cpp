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
// #include <cctype>
// #include <stdexcept>
#include <sstream>
#include <iostream>
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
	*history << "test 1" << std::endl;
	*history << "test 2" << std::endl;
	*history << "test 3" << std::endl;
	*history << "test 4" << std::endl;
	*history << "test 5" << std::endl;
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

	char c;
	while ((c = std::getchar()) != EOF)
	{
		// escape sequence
		if (c == 27)
		{
			while (! std::isalpha(c))
			{
				c = std::getchar();
			}
			switch (c)
			{
				// KeyUp
				// history previous
				case 65:
					{
						std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
						is_buf->str("");
						is->clear();

						if (history_ln - 1 < 0)
						{
							history->clear();
						}
						string line;
						if (std::getline(*history, line))
						{
							history_ln++;

							std::printf("\r> ");
							for (int i = 0; i != len; i++)
								std::putchar(' ');

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
						else
						{
							tty_bell();
						}
						
					}
					continue;
				break;
				// KeyDown
				// history next
				case 66:
					{
						std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
						is_buf->str("");
						is->clear();

						if (history_ln - 1 > 0)
						{
							history->clear();
							history->seekg(0);

							for (int i = 0; i != history_ln - 2; i++)
							{
								history->ignore(std::numeric_limits<std::streamsize>::max(), history->widen('\n'));
							}
						}
						else
						{
							history->seekg(EOF);
						}
						string line;
						if (std::getline(*history, line))
						{
							history_ln--;

							std::printf("\r> ");
							for (int i = 0; i != len; i++)
								std::putchar(' ');

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
						else
						{
							std::printf("\r> ");
							cur = len = 0;
							tty_bell();
						}
						
					}
					continue;
				break;
				// KeyRight
				// move cursor right
				case 67: // key Right
					if (cur <= len && len)
					{
						tty_gotoright();
						cur++;
					}
					else
					{
						tty_bell();
					}
					continue;
				break;
				// KeyLeft
				// move cursor left
				case 68:
					if (cur && len)
					{	
						tty_gotoleft();
						cur--;
					}
					else
					{
						tty_bell();
					}
					continue;
				break;
			}
		}
		// KeyDel
		// delete char
		else if (c == 127)
		{
			if (cur && len)
			{
				tty_erase();
				cur--, len--;
			}
			else
			{
				tty_bell();
			}
			continue;
		}
		// KeyReturn
		// stdin release
		else if (c == 10)
		{
			std::putchar(c);
			*history << c;

			history->clear(); // reset stream state (pos)
			history->seekg(0);

			break;
		}

		// std::printf("char: '%c'\n", c);

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
