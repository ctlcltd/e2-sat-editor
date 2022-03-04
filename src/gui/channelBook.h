/*!
 * e2-sat-editor/src/gui/channelBook.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

using namespace std;

#ifndef channelBook_h
#define channelBook_h
#include <QWidget>
#include <QHBoxLayout>
#include <QListWidget>
#include "../e2db.h"

namespace e2se_gui
{
class channelBook
{
    public:
    	channelBook(e2db* dbih);
		void side();
		void stacked();
		void sideRowChanged(int index);
		void populate(int vv);
		QWidget* widget;
	protected:
		QListWidget* lwid;
		QHBoxLayout* swid;
		QTreeWidget* list;
		QTreeWidget* tree;
		QTabBar* tabv;
		enum views {
			Services,
			Bouquets,
			Satellites,
			Providers,
			Resolution,
			Encryption,
			A_Z
		};
	private:
		e2db* dbih;
		int flag;
};
}
#endif
