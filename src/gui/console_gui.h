/*!
 * e2-sat-editor/src/gui/console_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>
#include <any>

using std::istream;

#ifndef console_gui_h
#define console_gui_h
#include <QApplication>
#include <QWidget>
#include <QTextStream>
#include <QByteArray>

#include "toolkit/ConsoleWidget.h"
#include "../logger/logger.h"
#include "../e2db/e2db_console.h"
#include "dataHandler.h"

namespace e2se_gui
{

class stream : public ::e2se_e2db::streamiface
{
	public:
		stream(QTextStream &p) : ts(p) {}
		virtual ~stream() = default;

		streamiface &operator<<(int i) override { ts << i; ts.setFieldWidth(0); return *this; }
		streamiface &operator<<(char c) override { ts << c; ts.setFieldWidth(0); return *this; }
		streamiface &operator<<(const char* s) override { ts << s; ts.setFieldWidth(0); return *this; }
		streamiface &operator<<(const std::string &s) override { ts << s.c_str(); ts.setFieldWidth(0); return *this; }
		streamiface &operator<<(const streamiface &) override { return *this; }

		streamiface &endl() override { ts << '\n'; ts.setFieldWidth(0); return *this; }
		streamiface &flush() override { ts.flush(); return *this; }
		streamiface &left() override { ts.setFieldAlignment(QTextStream::AlignLeft); return *this; }
		streamiface &right() override { ts.setFieldAlignment(QTextStream::AlignRight); return *this; }
		int width() const override { return ts.fieldWidth(); }
		int width(int width) override { ts.setFieldWidth(width); return ts.fieldWidth(); }

	private:
		QTextStream &ts;
};


class console_gui : protected e2se::log_factory, public ::e2se_e2db::e2db_console
{
	Q_DECLARE_TR_FUNCTIONS(console_gui)

	public:

		enum EVENT {
			cmd_fread = COMMAND::fread,
			cmd_fwrite = COMMAND::fwrite,
			cmd_fimport = COMMAND::fimport,
			cmd_fexport = COMMAND::fexport,
			cmd_merge = COMMAND::merge,
			cmd_parse = COMMAND::parse,
			cmd_make = COMMAND::make,
			cmd_convert = COMMAND::convert,
			cmd_tool = COMMAND::tool,
			cmd_macro = COMMAND::macro,
			cmd_tab_reload,
			cmd_tab_clear
		};

		console_gui(QWidget* parent, dataHandler* data);
		~console_gui(); // final destructor
		void close();
		void attach(QWidget* parent);
		void detach();
		void destroy();

		void setEventCallback(std::function<void(EVENT event)> func)
		{
			this->eventCallback = func;
		}

	protected:

		struct current {
			ENTRY entry_type;
			bool edit;
			string id;
			int ref;
			string bname;
			int i = 0;
			int pos = 0;
			bool end = false;
			TYPE type;
			bool required;
			std::any val;
			map<int, vector<pair<TYPE, bool>>> mask;
			map<TYPE, string> values;
		};

		struct nav {
			ENTRY entry_type;
			int limit;
			int rows;
			string bname;
			int pos = 0;
			int offset = 0;
			int end = 0;
		};

		void init();
		void layout(QWidget* parent);
		void session();
		void sync();
		void prompt();
		void flush();
		void clear();

		void console_resolver(COMMAND command, istream* is) override;
		void console_usage(COMMAND hint, int level = 3) override;
		void console_print(int opt) override;

		void command_help(istream* is) { console_resolver(COMMAND::usage, is); }
		void command_preferences(istream* is) { console_resolver(COMMAND::preferences, is); }
		void command_clear(istream* is) { console_resolver(COMMAND::clear, is); }
		void command_tab(istream* is) { console_resolver(COMMAND::tab, is); }
		void command_quit();

		void entry_list(ENTRY entry_type, bool paged = true, int limit = 0, int pos = 0, string bname = "") override;
		void entry_edit(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "") override;
		std::any field(TYPE type, bool required = false) override;
		
		void input_step(current &curr);
		void input_next(current &curr);
		void input_end(current &curr);
		void input_end();
		void paged_nav(nav &p);
		void paged_end();

		void e2db_log();
		void demo_message();

		void callEventCallback(EVENT event)
		{
			if (this->eventCallback)
				this->eventCallback(event);
		}

		ConsoleWidget* cnt = nullptr;
		dataHandler* data = nullptr;
		current* icurr = nullptr;

	private:
		QByteArray* ba_out = nullptr;
		QByteArray* ba_err = nullptr;
		QTextStream* ts_out = nullptr;
		QTextStream* ts_err = nullptr;
		size_t logpos = 0;
		std::function<void(EVENT event)> eventCallback;
};

}
#endif /* console_gui_h */
