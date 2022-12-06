/*!
 * e2-sat-editor/src/gui/tunersets.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tunersets_h
#define tunersets_h
#include <QWidget>
#include <QTreeWidget>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class tunersets : protected e2se::log_factory
{
	public:
		tunersets(e2db* dbih, int ytype, e2se::logger::session* log);
		void layout();
		void treeItemChanged();
		void load();
		void populate();
		void trickySortByColumn(int column);
		QWidget* widget;
		QSettings* sets;
	protected:
		QTreeWidget* list;
		QTreeWidget* tree;
	private:
		e2db* dbih;
		// tunersets type
		int yx;
};
}
#endif /* tunersets_h */
