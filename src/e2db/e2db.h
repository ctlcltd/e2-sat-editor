/*!
 * e2-sat-editor/src/e2db/e2db.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../logger/logger.h"
#include "e2db_parser.h"
#include "e2db_maker.h"
#include "e2db_converter.h"

#ifndef e2db_h
#define e2db_h
namespace e2se_e2db
{
class e2db : public e2db_parser, public e2db_maker, public e2db_converter
{
	public:
		explicit e2db();
		e2db(e2se::logger::data* obj);
		virtual ~e2db() = default;
		void import_file(vector<string> paths);
		void import_file(FPORTS fpi, e2db* dst, e2db_file file, string path);
		void export_file(vector<string> paths);
		void export_file(FPORTS fpo, vector<string> paths);
		void export_file(FPORTS fpo, string path);
		void add_transponder(transponder& tx);
		void edit_transponder(string txid, transponder& tx);
		void remove_transponder(string txid);
		void add_service(service& ch);
		void edit_service(string chid, service& ch);
		void remove_service(string chid);
		void add_bouquet(bouquet& bs);
		void edit_bouquet(bouquet& bs);
		void remove_bouquet(string bname);
		void add_userbouquet(userbouquet& ub);
		void edit_userbouquet(userbouquet& ub);
		void remove_userbouquet(string bname);
		void add_channel_reference(channel_reference& chref, string bname);
		void edit_channel_reference(string chid, channel_reference& chref, string bname);
		void remove_channel_reference(channel_reference chref, string bname);
		void remove_channel_reference(string chid, string bname);
		void add_tunersets(tunersets& tv);
		void edit_tunersets(int tvid, tunersets& tv);
		void remove_tunersets(int tvid);
		void add_tunersets_table(tunersets_table& tn, tunersets tv);
		void edit_tunersets_table(string tnid, tunersets_table& tn, tunersets tv);
		void remove_tunersets_table(string tnid, tunersets tv);
		void add_tunersets_transponder(tunersets_transponder& tntxp, tunersets_table tn);
		void edit_tunersets_transponder(string trid, tunersets_transponder& tntxp, tunersets_table tn);
		void remove_tunersets_transponder(string trid, tunersets_table tn);
		string get_filepath();
		string get_services_filename();
		map<string, vector<pair<int, string>>> get_channels_index();
		map<string, vector<pair<int, string>>> get_transponders_index();
		map<string, vector<pair<int, string>>> get_services_index();
		map<string, vector<pair<int, string>>> get_bouquets_index();
		map<string, vector<pair<int, string>>> get_userbouquets_index();
		map<string, vector<pair<int, string>>> get_packages_index();
		map<string, vector<pair<int, string>>> get_resolution_index();
		map<string, vector<pair<int, string>>> get_encryption_index();
		map<string, vector<pair<int, string>>> get_az_index();
		void merge(e2db_abstract* dst);

	protected:
		virtual e2db* newptr() { return new e2db(this->log->obj); }
};
}
#endif /* e2db_h */
