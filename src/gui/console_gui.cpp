/*!
 * e2-sat-editor/src/gui/console_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>
#include <iostream>

#include <QGridLayout>

#include "toolkit/DialogDockWidget.h"
#include "termctl_gui.h"
#include "console_gui.h"

using std::istream, std::string;

namespace e2se_gui
{

console_gui::console_gui(QWidget* parent, dataHandler* data)
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("gui", "console_gui");
	this->data = data;

	layout(parent);
	init();
}

//TODO TEST
// note: ctn widget deleted 0x0 by parent widget tab view widget
console_gui::~console_gui()
{
	debug("~console_gui");

	delete this->pout;
	delete this->perr;
	delete this->ts_out;
	delete this->ts_err;
	delete this->ba_out;
	delete this->ba_err;

	//TODO FIX SEGFAULT
	// delete this->termctl;
}

void console_gui::layout(QWidget* parent)
{
	debug("layout");

	QGridLayout* frm = new QGridLayout;
	this->cnt = new ConsoleWidget;

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addWidget(cnt, 0, 0);

	if (DialogDockWidget* dwid = qobject_cast<DialogDockWidget*>(parent))
		dwid->setLayout(frm);
	else
		frm->setParent(parent);

	parent->setProperty("console_widget", true);
}

void console_gui::init()
{
	debug("init");

	this->ba_out = new QByteArray;
	this->ba_err = new QByteArray;
	this->ts_out = new QTextStream(this->ba_out);
	this->ts_err = new QTextStream(this->ba_err);

	pout = new stream(*this->ts_out);
	perr = new stream(*this->ts_err);

	history = HISTORY::file;

	if (__objio.out == OBJIO::byline)
		__objio.hrn = false;
	else if (__objio.out == OBJIO::json)
		__objio.hrn = false;

	this->plog = this->log;
	this->termctl = new termctl_gui(this->cnt);

	console_header();
	flush();

	prompt();
}

void console_gui::close()
{
	debug("close");

	if (cnt->parentWidget() != nullptr && cnt->parentWidget()->parentWidget() != nullptr)
	{
		QWidget* widget = cnt->parentWidget()->parentWidget();

		if (widget != nullptr && ! widget->property("console_widget").isNull())
		{
			detach();
			widget->close();
		}
	}
}

void console_gui::attach(QWidget* parent)
{
	debug("attach");

	this->ba_out = new QByteArray;
	this->ba_err = new QByteArray;
	this->ts_out = new QTextStream(this->ba_out);
	this->ts_err = new QTextStream(this->ba_err);

	pout = new stream(*this->ts_out);
	perr = new stream(*this->ts_err);

	this->plog = this->log;
	//TODO QObject is connected, std::stringbuf SEGFAULT
	// this->termctl = new termctl_gui(this->cnt);

	QGridLayout* frm = new QGridLayout;
	frm->setContentsMargins(0, 0, 0, 0);
	frm->addWidget(cnt, 0, 0);

	if (DialogDockWidget* dwid = qobject_cast<DialogDockWidget*>(parent))
		dwid->setLayout(frm);
	else
		frm->setParent(parent);

	cnt->attachWidget();

	parent->setProperty("console_widget", 1);

	session();
}

void console_gui::detach()
{
	debug("detach");

	delete this->pout;
	delete this->perr;
	delete this->ts_out;
	delete this->ts_err;
	delete this->ba_out;
	delete this->ba_err;

	//TODO QObject is connected, std::stringbuf SEGFAULT
	termctl->reset();
	// delete this->termctl;

	this->plog = nullptr;
	this->pout = nullptr;
	this->perr = nullptr;
	this->ts_out = nullptr;
	this->ts_err = nullptr;
	this->ba_out = nullptr;
	this->ba_err = nullptr;
	// this->termctl = nullptr;

	cnt->detachWidget();
}

void console_gui::session()
{
	debug("session");

	termctl->reset();
	cnt->printSessionRuler();

	prompt();
}

void console_gui::sync()
{
	if (this->termctl != nullptr && this->dbih != nullptr)
	{
		if (this->dbih != this->data->dbih)
			session();
	}

	this->dbih = this->data->dbih;
}

void console_gui::prompt()
{
	auto* termctl = reinterpret_cast<termctl_gui*>(this->termctl);

	termctl->handler(termctl_gui::HANDLE::Command);

	cnt->printPromptCursor();

	termctl->input([=]() {
		string cmd = termctl->token();
		istream* is = termctl->ptr();
		termctl->clear();

		this->sync();

		if (cmd == "quit" || cmd == "exit" || cmd == "q")
			return command_quit();
		else if (cmd == "help" || cmd == "h")
			command_help(is);
		else if (cmd == "version" || cmd == "v")
			command_version();
		else if (cmd == "read" || cmd == "i")
			command_read(is);
		else if (cmd == "write" || cmd == "o")
			command_write(is);
		else if (cmd == "list" || cmd == "l")
			command_list(is);
		else if (cmd == "add" || cmd == "a")
			command_add(is);
		else if (cmd == "edit" || cmd == "e")
			command_edit(is);
		else if (cmd == "remove" || cmd == "r")
			command_remove(is);
		// else if (cmd == "copy" || cmd == "c")
		// 	command_copy(is);
		// else if (cmd == "move" || cmd == "m")
		// 	command_move(is);
		else if (cmd == "set" || cmd == "s")
			command_set(is);
		else if (cmd == "unset" || cmd == "u")
			command_unset(is);
		else if (cmd == "print" || cmd == "p")
			command_print(is);
		else if (cmd == "import")
			command_import(is);
		else if (cmd == "export")
			command_export(is);
		else if (cmd == "merge")
			command_merge(is);
		else if (cmd == "parse")
			command_parse(is);
		else if (cmd == "make")
			command_make(is);
		else if (cmd == "convert")
			command_convert(is);
		else if (cmd == "tool")
			command_tool(is);
		else if (cmd == "macro")
			command_macro(is);
		else if (cmd == "inspect")
			command_inspect(is);
		else if (cmd == "preferences")
			command_preferences(is);
		else if (! cmd.empty())
			console_error(cmd);

		flush();

		if (! cnt->isInputMasked())
			cnt->printPromptCursor();

		cnt->setInputMasked(false);

		delete is;
	});
}

void console_gui::flush()
{
	ts_err->seek(0);
	ts_out->seek(0);

	if (! ts_err->readAll().isEmpty())
	{
		ts_err->seek(0);
		cnt->printErrors(ts_err->readAll());
	}

	cnt->printOutput(ts_out->readAll());

	ba_err->clear();
	ba_out->clear();
	ts_err->seek(0);
	ts_out->seek(0);
}

void console_gui::console_resolver(COMMAND command, istream* is)
{
	if (command == COMMAND::preferences)
	{
		string src, type, opt1;
		*is >> std::skipws >> src >> type >> opt1;

		if (type.empty() || type == "output")
			console_preferences(type, opt1);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else
	{
		this->e2db_console::console_resolver(command, is);
	}
}

void console_gui::console_usage(COMMAND hint, int level)
{
	if (hint == COMMAND::preferences)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "preferences", *pout << ' ' << "CLI preferences." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "preferences", *pout << ' ';
			pout->width(24), *pout << pout->left() << "output  [format]", *pout << ' ' << "Output format (tabular, byline, json)" << pout->endl();
			*pout << pout->endl();
		}
	}
	else
	{
		this->e2db_console::console_usage(hint, level);
	}
}

void console_gui::command_quit()
{
	if (cnt->parentWidget() != nullptr)
		close();
}

void console_gui::entry_list(ENTRY entry_type, bool paged, int limit, int pos, string bname)
{
	int offset = limit;
	int end = 0;
	int rows = 1;

	//TODO FIX
	if (__objio.out == OBJIO::byline)
	{
		switch (entry_type)
		{
			case ENTRY::transponder: rows = 32; break;
			case ENTRY::service: rows = 24; break;
			case ENTRY::bouquet: rows = 6; break;
			case ENTRY::userbouquet: rows = 1; break;
			case ENTRY::tunersets: rows = 10; break;
			case ENTRY::tunersets_table: rows = 5; break;
			case ENTRY::tunersets_transponder: rows = 29; break;
			case ENTRY::channel_reference: rows = 1; break;
			default: rows = 1;
		}
	}
	else if (__objio.out == OBJIO::tabular)
	{
		rows = 5;
	}
	else if (__objio.out == OBJIO::json)
	{
		rows = 4;
	}

	if (paged)
	{
		nav p;
		p.entry_type = entry_type;
		p.limit = limit;
		p.rows = rows;
		p.bname = bname;
		p.pos = pos;
		p.offset = offset;
		p.end = end;

		if (limit == 0)
		{
			auto screensize = termctl->screensize();
			offset = screensize.first ? screensize.first / rows : 1;
		}

		auto* termctl = reinterpret_cast<termctl_gui*>(this->termctl);

		termctl->handler(termctl_gui::HANDLE::Listing);

		entry_list_exec(entry_type, pos, offset, end, bname);
		flush();

		// qDebug() << "entry_list" << "end:" << p.end;

		cnt->printNavigationRuler();

		termctl->input([=]() mutable {
			this->paged_nav(p);
		});

		cnt->setInputMasked(true);
	}
	else
	{
		entry_list_exec(entry_type, pos, offset, end, bname);
		flush();
	}
}

void console_gui::entry_edit(ENTRY entry_type, bool edit, string id, int ref, string bname)
{
	auto mask = input_mask(entry_type, edit, id, ref, bname);

	flush();

	current curr;
	curr.entry_type = entry_type;
	curr.edit = edit;
	curr.id = id;
	curr.ref = ref;
	curr.bname = bname;
	curr.mask = mask;

	auto* termctl = reinterpret_cast<termctl_gui*>(this->termctl);

	termctl->handler(termctl_gui::HANDLE::Input);

	input_next(curr);

	termctl->input([=]() mutable {
		string str = termctl->line();
		termctl->clear();

		// qDebug() << "str:[" << str << "]";

		if (this->value_field(curr.type, str, curr.required, curr.val))
		{
			// qDebug() << "emplace" << "type:" << curr.type << " str:[" << str << "]";

			curr.values.emplace(curr.type, str);

			curr.pos++;
		}

		this->input_next(curr);
	});

	cnt->setInputMasked(true);
}

std::any console_gui::field(TYPE type, bool required)
{
	// qDebug() << "field:" << type;

	if (this->icurr == nullptr)
		return -1;

	auto& values = this->icurr->values;

	if (values.count(type))
	{
		string str = values.at(type);
		std::any val;

		if (value_field(type, str, required, val))
			return val;
	}

	return -1;
}

void console_gui::input_step(current &curr)
{
	ENTRY &entry_type = curr.entry_type;
	int &i = curr.i;
	int &pos = curr.pos;
	bool &end = curr.end;
	auto &values = curr.values;

	end = true;

	if (entry_type == ENTRY::transponder)
	{
		if (i == 0 && values.count(TYPE::yname))
		{
			string str = values.at(TYPE::yname);
			end = false;
			if (str == "s") i = 1;
			else if (str == "t") i = 2;
			else if (str == "c") i = 3;
			else if (str == "a") i = 4;
			pos = 0;
		}
		else if (i > 0 && values.count(TYPE::txdata))
		{
			string str = values.at(TYPE::txdata);

			if (str != "N" && str != "n")
			{
				string str = values.at(TYPE::yname);
				end = false;
				if (str == "s") i = -1;
				else if (str == "t") i = -2;
				pos = 0;
			}
		}
	}
	else if (entry_type == ENTRY::service)
	{
		if (i == 0 && values.count(TYPE::txid))
		{
			string txid = values.at(TYPE::txid);

			this->sync();

			if (this->dbih->db.transponders.count(txid))
			{
				end = false;
				i = 1;
				pos = 0;
			}
		}
		else if (i == 1 && values.count(TYPE::chdata))
		{
			string str = values.at(TYPE::chdata);

			if (str != "N" && str != "n")
			{
				end = false;
				i = -1;
				pos = 0;
			}
		}
	}
	else if (entry_type == ENTRY::tunersets_table)
	{
		if (i == 0 && values.count(TYPE::ytype))
		{
			string str = values.at(TYPE::ytype);
			end = false;
			if (str == "s") i = 1;
			else if (str == "t") i = 2;
			else if (str == "c") i = 3;
			else if (str == "a") i = 4;
			else end = true;
			pos = 0;
		}
	}
	else if (entry_type == ENTRY::tunersets_transponder)
	{
		if (i == 0)
		{
			end = false;
			i = 1;
			pos = 0;
		}
		else if (i != -1 && values.count(TYPE::txdata))
		{
			string str = values.at(TYPE::txdata);

			if (str != "N" && str != "n")
			{
				string str = values.at(TYPE::yname);
				end = false;
				i = -1;
				pos = 0;
			}
		}
	}
	else if (entry_type == ENTRY::channel_reference)
	{
		if (i != -1 && values.count(TYPE::ffdata))
		{
			string str = values.at(TYPE::ffdata);

			if (str != "N" && str != "n")
			{
				end = false;
				i = -1;
				pos = 0;
			}
		}
	}
}

void console_gui::input_next(current &curr)
{
	qDebug() << "input_next" << "pos:" << curr.pos << "i:" << curr.i;

	auto &mask = curr.mask;
	int &i = curr.i;
	int &pos = curr.pos;
	bool &end = curr.end;
	TYPE &type = curr.type;
	bool &required = curr.required;

	if (end || (! mask.count(i) && ! mask.at(i).size()))
		return input_end(curr);

	auto props = mask.at(i);
	auto it = props.begin();
	auto last = props.end();

	if (it + pos != last)
	{
		it += pos;

		type = it->first;
		required = it->second;
	}
	else
	{
		input_step(curr);

		if (end)
		{
			return input_end(curr);
		}
		else if (mask.count(i) && mask.at(i).size())
		{
			auto props = mask.at(i);
			auto it = props.begin();
			auto last = props.end();

			if (it + pos != last)
			{
				it += pos;

				type = it->first;
				required = it->second;
			}
			else
			{
				return input_end(curr);
			}
		}
	}

	string label;
	string description;

	if (! label_field(type, label, description))
		return;

	this->curr_field = label;

	*pout << label;
	if (! description.empty())
		*pout << ' ' << '(' << description << ')';
	if (required)
		*pout << ' ' << '*';
	*pout << ':' << ' ';

	flush();
}

void console_gui::input_end(current &curr)
{
	qDebug() << "input_end";

	this->icurr = &curr;
	this->sync();

	entry_edit_exec(curr.entry_type, curr.edit, curr.id, curr.ref, curr.bname);

	this->icurr = nullptr;

	cnt->setInputMasked(false);
	flush();
	prompt();
}

void console_gui::paged_nav(nav &p)
{
	qDebug() << "paged_nav" << "pos:" << p.pos << "offset:" << p.offset;

	ENTRY entry_type = p.entry_type;
	int limit = p.limit;
	int rows = p.rows;
	string bname = p.bname;
	int &pos = p.pos;
	int &offset = p.offset;
	int &end = p.end;

	// qDebug() << "paged_nav-1" << "end:" << p.end;

	if (end)
		return paged_end();

	if (limit == 0)
	{
		auto screensize = termctl->screensize();
		offset = screensize.first ? screensize.first / rows : 1;
	}

	int key = termctl->paged(pos, offset);

	// qDebug() << "paged_nav-2" << "key:" << key;

	switch (key)
	{
		case termctl_gui::EVENT::InputEnd: return paged_end();
		case termctl_gui::EVENT::PagePrev: pos -= offset; break;
		case termctl_gui::EVENT::PageNext: pos += offset; break;
		default: return; // any key
	}

	this->sync();

	entry_list_exec(entry_type, pos, offset, end, bname);
	flush();

	// qDebug() << "paged_nav-3" << "end:" << p.end;

	cnt->printNavigationRuler();
}

void console_gui::paged_end()
{
	qDebug() << "paged_end";

	cnt->setInputMasked(false);
	prompt();
}

void console_gui::destroy()
{
	debug("destroy");

	delete this;
}

}
