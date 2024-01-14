/*!
 * e2-sat-editor/src/gui/e2db_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
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

#include "../e2db/e2db.h"

namespace e2se_gui
{
class e2db : public ::e2se_e2db::e2db
{
	Q_DECLARE_TR_FUNCTIONS(e2db)

	public:
		struct entry
		{
			unordered_map<string, QStringList> transponders;
			unordered_map<string, QStringList> services;
		} entries;

		e2db();
		~e2db(); // final destructor
		void didChange();
		void cache(bool clear = false);
		void fixBouquets();
		string addTransponder(transponder& tx);
		string editTransponder(string txid, transponder& tx);
		void removeTransponder(string txid);
		string addService(service& ch);
		string editService(string chid, service& ch);
		void removeService(string chid);
		string addBouquet(bouquet& bs);
		string editBouquet(bouquet& bs);
		void removeBouquet(string bname);
		string addUserbouquet(userbouquet& ub);
		string editUserbouquet(userbouquet& ub);
		void removeUserbouquet(string bname);
		string addChannelReference(channel_reference& chref, string bname);
		string editChannelReference(string chid, channel_reference& chref, string bname);
		void removeChannelReference(channel_reference chref, string bname);
		void removeChannelReference(string chid, string bname);
		int addTunersets(tunersets& tv);
		int editTunersets(int tvid, tunersets& tv);
		void removeTunersets(int tvid);
		string addTunersetsTable(tunersets_table& tn, tunersets tv);
		string editTunersetsTable(string tnid, tunersets_table& tn, tunersets tv);
		void removeTunersetsTable(string tnid, tunersets tv);
		string addTunersetsTransponder(tunersets_transponder& tntxp, tunersets_table tn);
		string editTunersetsTransponder(string trid, tunersets_transponder& tntxp, tunersets_table tn);
		void removeTunersetsTransponder(string trid, tunersets_table tn);
		void setServiceParentalLock(string chid);
		void unsetServiceParentalLock(string chid);
		void setUserbouquetParentalLock(string bname);
		void unsetUserbouquetParentalLock(string bname);
		bool prepare(string path);
		bool write(string path);
		void importFile(vector<string> paths);
		void exportFile(int flags, vector<string> paths);
		void importBlob(unordered_map<string, e2db_file> files, bool threading = false);
		QStringList entryTransponder(transponder tx);
		QStringList entryTransponder(transponder tx, bool extended);
		QStringList entryService(service ch);
		QStringList entryMarker(channel_reference chref);
		QStringList entryTunersetsTable(tunersets_table tn);
		QStringList entryTunersetsTransponder(tunersets_transponder tntxp, tunersets_table tn);
		static QString fixUnicodeChars(string str);
		void showError(string str);

	protected:
		e2db* newptr() override { return new e2se_gui::e2db; }
		void setup();
		void primer();

		string msg(string str, string param) override;

		void error(string msg, string optk, string optv) override;
		bool threading = false;
};
}
#endif /* e2db_gui_h */
