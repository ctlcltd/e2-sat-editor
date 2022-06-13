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
		e2db();
		virtual ~e2db() = default;
		void merge(e2db* dbih);
		void import_file();
		void export_file();
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
