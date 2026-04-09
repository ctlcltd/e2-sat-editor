/*!
 * e2-sat-editor/src/gui/console_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <any>

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

//TODO
class console_gui : protected e2se::log_factory, public ::e2se_e2db::e2db_console
{
	Q_DECLARE_TR_FUNCTIONS(console_gui)

	public:
		console_gui(QWidget* parent, dataHandler* data);
		~console_gui(); // final destructor
		void layout(QWidget* parent);

	protected:

		struct current {
			ENTRY entry_type = ENTRY::service;
			bool edit = false;
			string id;
			int ref;
			string bname;
			int i = 0;
			int pos = 0;
			bool end = false;
			TYPE type = TYPE::idx;
			bool required = false;
			std::any val;
			map<int, vector<pair<TYPE, bool>>> mask;
			map<TYPE, string> values;
		};

		struct nav {
			ENTRY entry_type = ENTRY::service;
			int limit;
			int rows;
			string bname;
			int pos = 0;
			int offset = 0;
			int end;
		};

		void sync();
		void output();
		void init();
		void prompt();

		void entry_list(ENTRY entry_type, bool paged = true, int limit = 0, int pos = 0, string bname = "") override;
		void entry_edit(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "") override;

		std::any field(TYPE type, bool required = false) override;
		void input_step(current &curr);
		void input_next(current &curr);
		void input_end(current &curr);
		void paged_nav(nav &p);
		void paged_end();

		ConsoleWidget* cnt = nullptr;
		dataHandler* data = nullptr;
		current* icurr = nullptr;

	private:
		QByteArray* ba_out = nullptr;
		QByteArray* ba_err = nullptr;
		QTextStream* ts_out = nullptr;
		QTextStream* ts_err = nullptr;
};

}
#endif /* console_gui_h */
