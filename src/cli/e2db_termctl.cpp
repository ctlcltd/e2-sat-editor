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

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(linux) || defined(__linux__) || defined(__APPLE__)
#define PLATFORM_UX
#endif

#if defined(WIN32) || defined(_WIN32)
#define PLATFORM_WIN
#endif

#ifdef PLATFORM_UX
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif
#ifdef PLATFORM_WIN
#include <windows.h>
#endif

#include "e2db_termctl.h"

namespace e2se_cli
{

#ifdef PLATFORM_UX
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

	*history << "read directory-not-exists" << std::endl;
	*history << "edit userbouquet id" << std::endl;
	*history << "add tunersets id" << std::endl;
	*history << "edit tunersets_transponder id" << std::endl;
	*history << "add transponder" << std::endl;
	*history << "edit service id" << std::endl;
	*history << "list transponders" << std::endl;
	*history << "list channels userbouquet.dbe01.tv" << std::endl;
	*history << "read e2se-seeds/enigma_db" << std::endl;

	this->last = this->history->tellg();
}

e2db_termctl::~e2db_termctl()
{
	reset();

	delete this->is;
}

void e2db_termctl::reset()
{
#ifdef PLATFORM_UX
	tty_set_sane();
#endif
}

void e2db_termctl::input(bool shell, bool ins)
{
#ifdef PLATFORM_UX
	tty_set_raw();
#endif

	if (shell)
	{
		std::printf("> ");
	}

	size_t cur, len;
	cur = len = 0;

	int prev, next;
	prev = next = 0;

	std::string input;
	char c;
	while ((c = std::getchar()) != EOF)
	{
		if (c == KEY_MAP::EscapeSequence)
		{
			while (! std::isalpha(c))
			{
				c = next = std::getchar();
			}
			switch (c)
			{
				case KEY_MAP::KeyUp:

					next = EVENT::HistoryBack;

					if (shell)
					{
						is->sync();
						std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

						std::streampos pos = history->tellg();

						// current input
						if (prev != EVENT::HistoryBack)
						{
							input = is_buf->str();
						}
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
							// history->clear();
							history->seekg(EOF);
							last = pos = EOF;
						}
						if (pos != EOF)
						{
							pos = 0;
							// history->clear();
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

							tty_eraseline(len);

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
					}

					prev = next;
					continue;
				break;
				case KEY_MAP::KeyDown:

					next = EVENT::HistoryForward;

					if (shell)
					{
						is->sync();
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
						else if (input != is_buf->str())
							line = input;
						else
							tty_bell();

						if (! line.empty() || pos != EOF)
						{
							is_buf->str("");
							is->clear();

							tty_eraseline(len);

							std::printf("\r> %s", line.c_str());
							cur = len = line.size();
							*is << line;
						}
					}

					prev = next;
					continue;
				break;
				case KEY_MAP::KeyRight:

					next = EVENT::CursorForward;

					if (cur < len && len != 0)
					{
						tty_goforward();

						cur++;

						is->seekp(0, std::ios_base::end);
						std::streamoff offset = cur - len;
						is->seekp(offset, std::ios_base::end);
					}
					else
					{
						tty_bell();
					}

					prev = next;
					continue;
				break;
				case KEY_MAP::KeyLeft:

					next = EVENT::CursorBackward;

					if (cur > 0 && len != 0)
					{
						tty_gobackward();

						cur--;

						is->seekp(0, std::ios_base::end);
						std::streamoff offset = cur - len;
						is->seekp(offset, std::ios_base::end);
					}
					else
					{
						tty_bell();
					}

					prev = next;
					continue;
				break;
			}
		}
		else if (c == KEY_MAP::KeyDelete)
		{
			next = EVENT::DeleteChar;

			if (cur > 0 && len != 0)
			{
				tty_delchar();

				std::streamoff offset = is->tellp();

				is->seekp(0, std::ios_base::beg);

				is->sync();
				std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

				std::string str = is_buf->str();
				try {
					str.erase(cur - 1, 1);
				} catch (...) {
					std::string str = is_buf->str();
					std::cerr << "out_of_range delete str:[" << str << "] cur:" << cur << " len:" << len << " size:" << str.size() << std::endl;
				}
				is_buf->str(str);
				is->sync();

				is->seekp(offset - 1);

				cur--, len--;
			}
			else
			{
				tty_bell();
			}

			prev = next;
			continue;
		}
		else if (c == KEY_MAP::KeyReturn)
		{
			next = EVENT::StdinRelease;

			is->seekp(0, std::ios_base::beg);

			is->sync();
			std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

			if (shell && ! is_buf->str().empty())
			{
				history->clear();
				history->seekp(0, std::ios_base::end);

				*history << is_buf->str();
				*history << std::endl;

				// history->clear();
				history->seekg(0);
				last = history->tellg();

				input = "";
			}

			std::putchar(c);

			is->seekp(0, std::ios_base::end);

			prev = next;
			break;
		}

		// std::printf("char: '%c'\n", c);

		// append
		if (! ins && cur != len)
		{
			std::streamoff offset = is->tellp();

			is->seekp(0, std::ios_base::beg);

			is->sync();
			std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());

			std::string str = is_buf->str();

			std::printf("%c%s", c, str.substr(cur).c_str());

			size_t n = len - cur;

			for (size_t i = 0; i != n; i++)
				std::putchar('\b');

			try {
				str.insert(cur, 1, c);
			} catch (...) {
				std::string str = is_buf->str();
				std::cerr << "out_of_range append str:[" << str << "] cur:" << cur << " len:" << len << " size:" << str.size() << std::endl;
			}

			is_buf->str(str);
			is->sync();

			is->seekp(offset + 1);
		}
		// insert | replace
		else
		{
			std::putchar(c);

			*is << c;
		}

		// insert
		if (ins && cur != len)
			cur++;
		// append | replace
		else
			cur++, len++;

		prev = next = 0;
	}

#ifdef PLATFORM_UX
	tty_set_sane();
#endif
}

const std::string e2db_termctl::str()
{
	std::string str;
	*is >> str;
	return str;
}

std::istream* e2db_termctl::stream()
{
	is->sync();

	std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
	std::stringbuf* cp_buf = new std::stringbuf;
	cp_buf->str(is_buf->str());

	return new std::istream(cp_buf);
}

void e2db_termctl::clear()
{
	std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
	is->clear();
	is_buf->str("");
}

int e2db_termctl::paged(int pos, int offset)
{
#ifdef PLATFORM_UX
	tty_set_raw();
#endif

	std::printf("Press key [Up] | [Down] to Move, [q] to Exit");

	int curr = 0;

	int c;
	while ((c = std::getchar()) != EOF)
	{
		if (c == KEY_MAP::EscapeSequence)
		{
			while (! std::isalpha(c))
			{
				c = curr = std::getchar();
			}
			switch (c)
			{
				case KEY_MAP::KeyUp:
					curr = EVENT::PagePrev;
				break;
				case KEY_MAP::KeyDown:
					curr = EVENT::PageNext;
				break;
				case KEY_MAP::KeyRight:
					curr = EVENT::PageNext;
				break;
				case KEY_MAP::KeyLeft:
					curr = EVENT::PagePrev;
				break;
				default:
					tty_bell();
			}
			if (curr != 0)
			{
				if (curr == EVENT::PagePrev)
				{
					if (pos - offset < 0)
					{
						tty_bell();

						continue;
					}
				}

				break;
			}
		}
		else if (c == KEY_MAP::KeyReturn)
		{
			curr = EVENT::PageNext;
			break;
		}
		else if (c == 'q' || c == 'Q' || c == 'x' || c == 'X')
		{
			std::putchar('\n');
			break;
		}
		else
		{
			tty_bell();
		}
	}

	tty_eraseline();

#ifdef PLATFORM_UX
	tty_set_sane();
#endif

	return curr;
}

std::pair<int, int> e2db_termctl::screensize()
{
	return e2db_termctl::tty_get_screensize();
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

#ifdef PLATFORM_UX
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

std::pair<int, int> e2db_termctl::tty_get_screensize()
{
#if defined(PLATFORM_WIN)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	return std::pair (csbiInfo.dwSize.Y, csbiInfo.dwSize.X);
#elif defined(PLATFORM_UX)
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return std::pair (w.ws_row, w.ws_col);
#else
	return std::pair (24, 80);
#endif
}

void e2db_termctl::tty_gotoxy(int x, int y)
{
#ifdef PLATFORM_WIN
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coordPos = {short (x), short (y)};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::printf("\e\x5b%d;%df", y, x);
#endif
}

void e2db_termctl::tty_goforward()
{
#ifdef PLATFORM_WIN
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	COORD coordPos = {csbiInfo.dwCursorPosition.X + 1, csbiInfo.dwCursorPosition.Y};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::printf("\e\x5b\1\x43");
#endif
}

void e2db_termctl::tty_gobackward()
{
#ifdef PLATFORM_WIN
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	COORD coordPos = {csbiInfo.dwCursorPosition.X - 1, csbiInfo.dwCursorPosition.Y};
	SetConsoleCursorPosition(hStdout, coordPos);
#else
	std::printf("\e\x5b\1\x44");
	// std::putchar('\b');
#endif
}

void e2db_termctl::tty_delchar()
{
	std::printf("\b\e\x5b\x50");
}

void e2db_termctl::tty_eraseline(int cols)
{
	std::printf("\r\b\e\x5b\2\x4b");
	// std::printf("\r  ");
	// for (int i = 0; i != cols; i++)
	// 	std::putchar(' ');
}

void e2db_termctl::tty_bell()
{
	std::putchar('\a');
}

}
