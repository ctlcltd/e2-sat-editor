/*!
 * e2-sat-editor/src/gui/convertM3u.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef convertM3u_h
#define convertM3u_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class convertM3u : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(convertM3u)

	public:
		enum DIAL {
			dial_import,
			dial_export
		};

		struct __state
		{
			DIAL dialog;
		};

		convertM3u(dataHandler* data);
		~convertM3u();
		void display(QWidget* cwid);
		void setImport(e2db::fcopts& opts);
		void setExport(e2db::fcopts& opts, vector<string>& ubouquets);
		e2db::fcopts getConverterOptions();

	protected:
		void layout(QWidget* cwid);
		void toolbarLayout();
		void importLayout();
		void exportLayout();
		void store();
		void retrieve();

		__state state;
		__action action;

	private:
		static QString browseFileDialog(QString path);

		e2db::fcopts opts;
		vector<string> ubouquets;
		QTreeWidget* ubt;
};
}
#endif /* convertM3u_h */
