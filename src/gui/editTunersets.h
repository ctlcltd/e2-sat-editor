/*!
 * e2-sat-editor/src/gui/editTunersets.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef editTunersets_h
#define editTunersets_h
#include <QWidget>
#include <QTreeWidget>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class editTunersets : protected e2se::log_factory
{
	public:
		editTunersets(e2db* dbih, int ytype);
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
#endif /* editTunersets_h */
