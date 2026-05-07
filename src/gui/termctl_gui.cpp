/*!
 * e2-sat-editor/src/gui/termctl_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cmath>
#include <limits>
#include <string>
#include <iostream>
#include <sstream>

#include <QApplication>

#include "termctl_gui.h"

namespace e2se_gui
{

termctl_gui::termctl_gui(ConsoleWidget* widget)
{
	this->widget = widget;

	std::stringbuf* is_buf = new std::stringbuf;
	this->is = new std::iostream(is_buf);

	std::stringbuf* history_buf = new std::stringbuf;
	this->history = new std::iostream(history_buf);

	// test
#if 0
	*history << "read directory-not-exists" << std::endl;
	*history << "edit userbouquet id" << std::endl;
	*history << "add tunersets id" << std::endl;
	*history << "edit tunersets_transponder id" << std::endl;
	*history << "add transponder" << std::endl;
	*history << "edit service id" << std::endl;
	*history << "list transponders" << std::endl;
	*history << "list channels userbouquet.dbe01.tv" << std::endl;
	*history << "read e2se-seeds/enigma_db" << std::endl;
#endif

	this->last = this->history->tellg();
}

termctl_gui::~termctl_gui()
{
	widget->disconnect(widget, &ConsoleWidget::input, nullptr, nullptr);

	this->commandCallback = nullptr;
	this->inputCallback = nullptr;

	//TODO FIX EXC_BAD_ACCESS SEGFAULT
	// delete this->is;

	delete this;
}

// note: callHandlerCallback is called asyncronously by QObject widget connected lambda function
void termctl_gui::callHandlerCallback(const int key, const QString val)
{
	if (this->currhr == HANDLE::Command && ! this->commandCallback)
		return;
	else if (this->currhr != HANDLE::Command && ! this->inputCallback)
		return;

	if (this->currhr == HANDLE::Listing)
	{
		this->currkey = key;

		this->inputCallback(1);
	}
	else
	{
		EVENT e = static_cast<termctl_gui::EVENT>(key);

		if (this->currhr == HANDLE::Command && key != 0)
			this->callHistory(e, val);
		else if (this->currhr == HANDLE::Input && e == EVENT::InputEnd)
			this->inputCallback(0);

		if (e == EVENT::InputReturn || key == 0)
		{
			*is << val.toStdString();

			if (this->currhr == HANDLE::Command)
				this->commandCallback();
			else
				this->inputCallback(1);
		}
	}
}

void termctl_gui::handler(HANDLE handle)
{
	this->currhr = handle;
	widget->setCurrentHandler(this->currhr);

	if (! this->connected)
	{
		widget->connect(widget, &ConsoleWidget::input, widget, [=](const int key, const QString str) {
			this->callHandlerCallback(key, str);
		});
		this->connected = true;
	}
}

void termctl_gui::clear()
{
	std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
	is->clear();
	is_buf->str("");
}

std::istream* termctl_gui::ptr()
{
	is->sync();

	std::stringbuf* is_buf = reinterpret_cast<std::stringbuf*>(is->rdbuf());
	std::stringbuf* cp_buf = new std::stringbuf;
	cp_buf->str(is_buf->str());

	return new std::istream(cp_buf);
}

const std::string termctl_gui::line()
{
	std::string str;
	std::getline(*is, str);
	return str;
}

const std::string termctl_gui::token()
{
	std::string str;
	std::getline(*is, str, ' ');
	return str;
}

void termctl_gui::reset()
{
	widget->disconnect(widget, &ConsoleWidget::input, nullptr, nullptr);
	this->connected = false;
}

void termctl_gui::callHistory(const EVENT e, const QString val)
{
	if (this->currhr != HANDLE::Command)
		return;

	int& prev = curr.prev;
	int& next = curr.next;
	std::string& input = curr.input;

	next = e;

	if (e == EVENT::InputReturn)
	{
		if (! val.isEmpty())
		{
			history->clear();
			history->seekp(0, std::ios_base::end);

			*history << val.toStdString();
			*history << std::endl;

			history->seekg(0);
			last = history->tellg();
		}

		input = "";
		next = 0;
	}
	else if (e == EVENT::HistoryPrev)
	{
		std::streampos pos = history->tellg();

		// current input
		if (prev != EVENT::HistoryPrev)
		{
			input = val.toStdString();
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
			history->seekg(EOF);
			last = pos = EOF;
		}
		if (pos != EOF)
		{
			pos = 0;
			history->seekg(pos);
			while (history->ignore(std::numeric_limits<std::streamsize>::max(), '\n'))
			{
				if (last)
					pos = history->tellg() != last ? history->tellg() : pos;
				else
					pos = history->tellg() != history->tellp() ? history->tellg() : pos;
				if (last == history->tellg() || history->tellp() == history->tellg())
					break;
			}
			history->clear();
			history->seekg(pos);
		}

		std::string line;
		if (std::getline(*history, line))
			last = pos;
		else
			QApplication::beep();

		if (! line.empty())
			widget->printHistory(static_cast<Qt::Key>(e), QString::fromStdString(line));
	}
	else if (e == EVENT::HistoryNext)
	{
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
		else if (input != val.toStdString())
			line = input;
		else
			QApplication::beep();

		if (! line.empty() || pos != EOF)
			widget->printHistory(static_cast<Qt::Key>(e), QString::fromStdString(line));
	}

	prev = next;
}

int termctl_gui::paged(int pos, int offset)
{
	int key = 0;

	switch (this->currkey)
	{
		case Qt::Key_Up:
			key = EVENT::PagePrev;
		break;
		case Qt::Key_Down:
			key = EVENT::PageNext;
		break;
		case Qt::Key_Right:
			key = QApplication::layoutDirection() == Qt::LeftToRight ? EVENT::PageNext : EVENT::PagePrev;
		break;
		case Qt::Key_Left:
			key = QApplication::layoutDirection() == Qt::LeftToRight ? EVENT::PagePrev : EVENT::PageNext;
		break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			key = EVENT::PageNext;
		break;
		case Qt::Key_Q:
		case Qt::Key_X:
		case Qt::Key_Escape:
		case Qt::Key_End:
			key = EVENT::InputEnd;
		default:
			QApplication::beep();
	}

	if (key != 0)
	{
		if (key == EVENT::PagePrev)
		{
			if (pos - offset < 0)
			{
				key = 0;

				QApplication::beep();
			}
		}
	}

	this->currkey = 0;

	return key;
}

std::pair<int, int> termctl_gui::screensize()
{
	QSize size = widget->size();
	int rows = 24;
	int cols = 80;

	if (size.height() != 0 && size.height() != 480)
		rows = int (std::floor(size.height() / 14.7));

	if (size.width() != 0 && size.width() != 640)
		cols = int (std::floor(size.width() / 7.2));

	return std::pair (rows, cols);
}

void termctl_gui::load_history()
{
	QApplication* app = qApp;

	QString hval = app->property("console_history").toString();

	if (! hval.isEmpty())
	{
		history->clear();
		history->seekg(0);

		std::stringbuf* h_buf = reinterpret_cast<std::stringbuf*>(history->rdbuf());
		h_buf->str(hval.toStdString());

		history->seekp(0, std::ios::end);

		last = history->tellg();
	}
}

void termctl_gui::save_history()
{
	QApplication* app = qApp;

	if (history->tellp() != 0)
	{
		history->clear();
		history->seekg(0);

		std::stringbuf* h_buf = reinterpret_cast<std::stringbuf*>(history->rdbuf());
		app->setProperty("console_history", QString::fromStdString(h_buf->str()));
	}
}

}
