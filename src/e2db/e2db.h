/*!
 * e2-sat-editor/src/e2db/e2db.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../logger/logger.h"
#include "e2db_parser.h"
#include "e2db_maker.h"

#ifndef e2db_h
#define e2db_h
namespace e2se_e2db
{
class e2db : public e2db_parser, public e2db_maker
{
	public:
		enum FPORTS {
			allServices = 0x1000,
			allServices2_2 = 0x1222,
			allServices2_3 = 0x1223,
			allServices2_4 = 0x1224,
			allServices2_5 = 0x1225,
			allBouquets = 0x2000,
			allUserbouquets = 0x4000,
			allTunersets = 0x8000,
			singleBouquet = 0x0002,
			singleBouquetAll = 0x0400,
			singleUserbouquet = 0x0004,
			singleTunersets = 0x0008,
			_default = 0x0000
		};

		// e2db(e2se::logger::session log);
		// e2db();
		virtual ~e2db() = default;
		void merge(e2db* dbih);
		void import_file(vector<string> paths);
		void import_file(FPORTS fpi, e2db* dbih, e2db_file file, string path);
		void export_file(vector<string> paths);
		void export_file(FPORTS fpo, vector<string> paths);
		void export_file(FPORTS fpo, string path);
		FPORTS filetype_detect(string path);
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
		string get_reference_id(string chid);
		string get_reference_id(channel_reference chref);
		map<string, vector<pair<int, string>>> get_channels_index();
		map<string, vector<pair<int, string>>> get_transponders_index();
		map<string, vector<pair<int, string>>> get_services_index();
		map<string, vector<pair<int, string>>> get_bouquets_index();
		map<string, vector<pair<int, string>>> get_userbouquets_index();
		map<string, vector<pair<int, string>>> get_packages_index();
		map<string, vector<pair<int, string>>> get_resolution_index();
		map<string, vector<pair<int, string>>> get_encryption_index();
		map<string, vector<pair<int, string>>> get_az_index();
};
}
#endif /* e2db_h */
