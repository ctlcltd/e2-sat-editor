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

#include <QApplication>

#include "termctl_gui.h"

namespace e2se_gui
{

termctl_gui::termctl_gui(ConsoleWidget* widget)
{
	this->widget = widget;

	std::stringbuf* is_buf = new std::stringbuf;
	this->is = new std::iostream(is_buf);
}

termctl_gui::~termctl_gui()
{
	widget->disconnect(widget, &ConsoleWidget::input, nullptr, nullptr);

	this->inputCallback = nullptr;

	//TODO FIX EXC_BAD_ACCESS SEGFAULT
	// delete this->is;

	delete this;
}

// note: callInputCallback is called asyncronously by QObject widget connected lambda function
void termctl_gui::callInputCallback(const int key, const QString val)
{
	if (! this->inputCallback)
		return;

	if (this->currhr == HANDLE::Listing)
	{
		this->currkey = key;

		this->inputCallback();
	}
	else
	{
		EVENT e = static_cast<termctl_gui::EVENT>(key);

		if (e == EVENT::InputReturn || key == 0)
		{
			*is << val.toStdString();

			this->inputCallback();
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
			this->callInputCallback(key, str);
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

}
