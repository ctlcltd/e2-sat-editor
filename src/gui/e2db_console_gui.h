/*!
 * e2-sat-editor/src/gui/e2db_console_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <sstream>
#include <string>
#include <vector>

#ifndef e2db_console_gui_h
#define e2db_console_gui_h
#include <QApplication>
#include <QWidget>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextStream>

#include "../logger/logger.h"
#include "../e2db/e2db_console.h"
#include "dataHandler.h"

namespace e2se_gui
{

//TODO
class termctl : public ::e2se_e2db::termiface
{
	public:
		termctl() {}
		virtual ~termctl() = default;
		void reset() {}
		void handler(bool command = false) {}
		std::istream* ptr() { return nullptr; }
		const std::string str() { return ""; }
		void clear() {}
		int paged(int pos, int offset) { return 0; }
		std::pair<int, int> screensize() { return {}; }
		void dump_log() {}
		void load_history() {}
		void save_history() {}
};

class stream : public ::e2se_e2db::streamiface
{
	public:
		stream(QTextStream &p) : ts(p) {}
		virtual ~stream() = default;

		streamiface &operator<<(int i) override { ts << i; return *this; }
		streamiface &operator<<(char c) override { ts << c; return *this; }
		streamiface &operator<<(const char* s) override { ts << s; return *this; }
		streamiface &operator<<(const std::string &s) override { ts << s.c_str(); return *this; }
		streamiface &operator<<(const streamiface &) override { return *this; }

		streamiface &endl() override { ts << '\n'; return *this; }
		streamiface &flush() override { ts.flush(); return *this; }
		streamiface &left() override { ts.setFieldAlignment(QTextStream::AlignLeft); return *this; }
		streamiface &right() override { ts.setFieldAlignment(QTextStream::AlignRight); return *this; }
		int width() const override { return ts.fieldWidth(); }
		int width(int width) override { ts.setFieldWidth(width); return ts.fieldWidth(); }

	private:
		QTextStream &ts;
};

//TODO
class ConsoleWidget : public QPlainTextEdit
{
	Q_OBJECT

	public:
		explicit ConsoleWidget(QWidget* parent = nullptr) : QPlainTextEdit(parent) {}

	signals:
		void command(const QString cmd);

	protected:
		void keyPressEvent(QKeyEvent* event) override
		{
			if (event->key() == Qt::Key_Return)
				emit command(document()->lastBlock().text().removeFirst());

			QPlainTextEdit::keyPressEvent(event);
		}
};

//TODO
class e2db_console_gui : protected e2se::log_factory, public ::e2se_e2db::e2db_console
{
	Q_DECLARE_TR_FUNCTIONS(e2db_console_gui)

	public:
		e2db_console_gui(QWidget* parent, dataHandler* data);
		~e2db_console_gui(); // final destructor
		void layout(QWidget* parent);

	protected:
		void init();

		ConsoleWidget* cnt = nullptr;
		dataHandler* data = nullptr;
};

}
#endif /* e2db_console_gui_h */
