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

#include <string>
#include <vector>
#include <unordered_map>

using std::string, std::pair, std::vector, std::unordered_map;

#ifndef e2db_gui_h
#define e2db_gui_h
#include <QApplication>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QSettings>

#include "../e2db.h"

namespace e2se_gui
{
class e2db : public ::e2se_e2db::e2db
{
	Q_DECLARE_TR_FUNCTIONS(e2db)

	public:
		struct entry {
			unordered_map<string, QStringList> transponders;
			unordered_map<string, QStringList> services;
		} entries;
		e2db();
		~e2db()
		{
			std::cout << "e2se_gui::~e2db()" << std::endl;
		}
		void options();
		string add_transponder(transponder& tx);
		string edit_transponder(string txid, transponder& tx);
		void remove_transponder(string txid);
		string add_service(service& ch);
		string edit_service(string chid, service& ch);
		void remove_service(string chid);
		void add_bouquet();
		void edit_bouquet();
		void remove_bouquet();
		void add_userbouquet();
		void edit_userbouquet();
		void remove_userbouquet();
		bool prepare(string localdir);
		bool write(string localdir, bool overwrite);
		void updateUserbouquetIndexes();
		void updateUserbouquetIndexes(string chid, string nw_chid);
		QStringList entry_transponder(transponder tx);
		QStringList entry_service(service ch);
		QStringList entry_marker(channel_reference chref);
		unordered_map<string, vector<pair<int, string>>> gindex;
	protected:
		QSettings* sets;
};
}
#endif /* e2db_gui_h */
