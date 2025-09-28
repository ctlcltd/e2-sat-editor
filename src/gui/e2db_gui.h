/*!
 * e2-sat-editor/src/gui/e2db_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

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
		struct storage
		{
			unordered_map<string, QStringList> transponders;
			unordered_map<string, QStringList> services;
		} entries;

		e2db();
		~e2db(); // final destructor
		void didChange();
		void initStorage();
		void clearStorage();
		void clearStorage(bool merge);
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
		bool prepare(string path) noexcept;
		bool write(string path) noexcept;
		void importFile(vector<string> paths) noexcept;
		void exportFile(int flags, vector<string> paths, string filename) noexcept;
		void importBlob(unordered_map<string, e2db_file> files);
		bool haveErrors();
		vector<string> getErrors();
		void clearErrors();
		QStringList entryTransponder(transponder tx);
		QStringList entryTransponder(transponder tx, bool extended);
		QStringList entryService(service ch);
		QStringList entryFavourite(channel_reference chref);
		QStringList entryMarker(channel_reference chref);
		QStringList entryTunersetsTable(tunersets_table tn);
		QStringList entryTunersetsTransponder(tunersets_transponder tntxp, tunersets_table tn);

		static QString fixUnicodeChars(string str);
		static QString doubleToSingleEscaped(QString text);

	protected:
		e2db* newptr() override { return new e2se_gui::e2db; }
		void setup();
		void primer();
		string msg(string str, string param) override;
		string msg(string str) override;
		void error(string msg, string optk, string optv) override;
		void trace(string error);

		void sort_compare(vector<pair<sort_data::value*, int>>& sorting, SORT_ORDER& order) override
		{
			const auto compare = (order == sort_asc ? &e2db::valueLessThan : &e2db::valueGreaterThan);
			std::stable_sort(sorting.begin(), sorting.end(), compare);
		}

		static bool valueLessThan(const pair<sort_data::value*, int>& left, const pair<sort_data::value*, int>& right)
		{
			switch (left.first->type)
			{
				case sort_data::integer: return left.first->val_integer() < right.first->val_integer();
				case sort_data::string: return QString::fromStdString(left.first->val_string()).localeAwareCompare(QString::fromStdString(right.first->val_string())) < 0;
				case sort_data::boolean: return left.first->val_boolean() < right.first->val_boolean();
				default: return false;
			}
		}
		static bool valueGreaterThan(const pair<sort_data::value*, int>& left, const pair<sort_data::value*, int>& right)
		{
			switch (right.first->type)
			{
				case sort_data::integer: return right.first->val_integer() < left.first->val_integer();
				case sort_data::string: return QString::fromStdString(right.first->val_string()).localeAwareCompare(QString::fromStdString(left.first->val_string())) < 0;
				case sort_data::boolean: return right.first->val_boolean() < left.first->val_boolean();
				default: return false;
			}
		}
};
}
#endif /* e2db_gui_h */
