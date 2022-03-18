/*!
 * e2-sat-editor/src/gui/e2db_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef e2db_gui_h
#define e2db_gui_h
#include <QString>
#include <QList>
#include <QRegularExpression>

#include "../commons.h"
#include "../e2db.h"

namespace e2se_gui
{
class e2db : public ::e2db::e2db
{
	public:
		struct entry {
			unordered_map<string, QStringList> transponders;
			unordered_map<string, QStringList> services;
		} entries;
		e2db();
		~e2db()
		{
			e2se::debug("e2db", "~destructor");
		}
		void options();
		bool prepare(string localdir);
		QStringList entry_transponder(transponder tx);
		QStringList entry_service(service ch);
		QStringList entry_marker(reference cref);
};
}
#endif /* e2db_gui_h */
