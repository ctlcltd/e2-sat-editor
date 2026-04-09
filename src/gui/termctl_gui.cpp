/*!
 * e2-sat-editor/src/gui/termctl_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "termctl_gui.h"

namespace e2se_gui
{

termctl_gui::termctl_gui(ConsoleWidget* widget)
{
	this->widget = widget;

	std::stringbuf* is_buf = new std::stringbuf;
	this->is = new std::iostream(is_buf);
}

void termctl_gui::callInputCallback(termctl_gui::EVENT key, const QString val)
{
	if (! this->inputCallback)
		return;

	if (key == termctl_gui::EVENT::InputReturn || key == termctl_gui::EVENT::InputEnter)
	{
		if (this->command)
			*is << QString(val).removeFirst().toStdString();
		else
			*is << val.toStdString();

		this->inputCallback();

		if (this->command)
			widget->prompt();
	}
}

void termctl_gui::handler(bool command)
{
	this->command = command;

	if (command)
		widget->prompt();

	if (! this->connected)
	{
		widget->connect(widget, &ConsoleWidget::input, widget, [=](Qt::Key key, const QString str) {
			this->callInputCallback(static_cast<termctl_gui::EVENT>(key), str);
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

const std::string termctl_gui::str()
{
	std::string str;
	*is >> str;
	return str;
}

void termctl_gui::reset()
{
	widget->disconnect(widget, &ConsoleWidget::input, nullptr, nullptr);
	this->connected = false;
}

int termctl_gui::paged(int pos, int offset)
{
	widget->nav("Press key [Up] | [Down] to Move, [q] to Exit");
}

}
