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
		void error(string cmsg, string optk, string optv);
		string addTransponder(transponder& tx);
		string editTransponder(string txid, transponder& tx);
		void removeTransponder(string txid);
		string addService(service& ch);
		string editService(string chid, service& ch);
		void removeService(string chid);
		void addBouquet(bouquet& bs);
		void editBouquet(bouquet& bs);
		void removeBouquet(string bname);
		string addUserbouquet(userbouquet& ub);
		string editUserbouquet(userbouquet& ub);
		void removeUserbouquet(string bname);
		void initialize();
		bool prepare(string localdir);
		bool write(string localdir, bool overwrite);
		void merge(unordered_map<string, e2se_e2db::e2db_file> files);
		QStringList entryTransponder(transponder tx);
		QStringList entryService(service ch);
		QStringList entryMarker(channel_reference chref);
		// reflect source index
		// gindex <name string, vector<pair<src-idx||count int, chid string>>>
		unordered_map<string, vector<pair<int, string>>> gindex;
	protected:
		QSettings* sets;
};
}
#endif /* e2db_gui_h */
