/*!
 * e2-sat-editor/src/gui/tools.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>

using std::string, std::vector;

#ifndef tools_h
#define tools_h
#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QMenu>

#include "../logger/logger.h"
#include "theme.h"
#include "dataHandler.h"

namespace e2se_gui
{
class gui;
class tab;

class tools : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(tools)

	public:

		enum INSPECT_FILTER {
			AllLog,
			Debug,
			Info,
			Error
		};

		enum SORT_ITEM {
			item_transponder,
			item_service,
			item_userbouquet,
			item_reference
		};

		struct sort_options
		{
			QString prop;
			int order;
			bool selecting;
			bool recall;
		};

		tools(tab* tid, gui* gid, QWidget* cwid, dataHandler* data);
		virtual ~tools();
		void inspector();
		void applyUtils(int bit, e2db::uoopts& opts, bool contextual = false);
		void execMacro(vector<string> pattern);
		void macroAutofix();
		void importFileCSV(e2db::FCONVS fci, e2db::fcopts opts);
		void exportFileCSV(e2db::FCONVS fco, e2db::fcopts opts);
		void exportFileHTML(e2db::FCONVS fco, e2db::fcopts opts);
		void importFileM3U(e2db::FCONVS fci, e2db::fcopts opts);
		void exportFileM3U(e2db::FCONVS fco, e2db::fcopts opts);
		void destroy();

	protected:
		QString inspectContent(string str, int filter = 0);
		void inspectUpdate(QTextEdit* view, int filter = 0);
		void inspectReset();
		void status(QString message);
		void done();
		bool showSortMenu(SORT_ITEM model, bool contextual, e2db::uoopts& opts);
		QMenu* sortMenu(SORT_ITEM model, bool contextual);
		void handleSortOptions(vector<QWidget*> fields);
		bool setupSortOptions(e2db::uoopts& opts);

		static vector<QPair<QString, QString>> sortComboBoxProps(SORT_ITEM model);

	private:
		e2se_gui::theme* theme;
		gui* gid;
		tab* tid;
		QWidget* cwid;
		dataHandler* data = nullptr;
		size_t inspect_pos = 0;
		INSPECT_FILTER inspect_curr;
		sort_options* sort_curr = nullptr;
};
}
#endif /* tools_h */
