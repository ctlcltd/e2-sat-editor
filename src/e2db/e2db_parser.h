/*!
 * e2-sat-editor/src/e2db/e2db_parser.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.4
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <fstream>

#include "../logger/logger.h"
#include "e2db_abstract.h"

using std::istream;

#ifndef e2db_parser_h
#define e2db_parser_h
namespace e2se_e2db
{
class e2db_parser : virtual public e2db_abstract
{
	public:

		inline static bool PARSER_LAMEDB5_PRIOR = false;
		inline static bool PARSER_TUNERSETS = true;
		inline static bool PARSER_PARENTALLOCK_LIST = true;

		e2db_parser();
		virtual ~e2db_parser() = default;
		void parse_e2db();
		void parse_e2db(unordered_map<string, e2db_file> files);
		void parse_e2db_lamedb(istream& ilamedb);
		void parse_e2db_lamedb5(istream& ilamedb);
		void parse_e2db_lamedbx(istream& ilamedb, int ver);
		void parse_e2db_bouquet(istream& ibouquet, string bname, bool epl = false);
		void parse_e2db_userbouquet(istream& iuserbouquet, string bname);
		void parse_e2db_parentallock_list(PARENTALLOCK ltype, istream& ilocked);
		void parse_userbouquet_reference(string str, userbouquet& ub);
		void parse_userbouquet_epl_reference(string str, userbouquet& ub);
		void parse_channel_reference(string str, channel_reference& chref, service_reference& ref);
		void parse_tunersets_xml(int ytype, istream& itunxml);
		void parse_zapit_services_xml(istream& iservicesxml);
		void parse_zapit_services_apix_xml(istream& iservicesxml, int ver);
		void parse_zapit_bouquets_apix_xml(istream& ibouquetsxml, int ver);
		bool find_services_file();
		bool list_file(string path);
		bool read(string path);
		unordered_map<string, string> get_input();

	protected:
		void parse_lamedb_transponder_params(string str, transponder& tx);
		void parse_lamedb_transponder_feparms(string str, char ty, transponder& tx);
		void parse_lamedb_service_params(string str, service& ch);
		void parse_lamedb_service_data(string str, service& ch);
		void append_lamedb_service_name(string str, service& ch);
};
}
#endif /* e2db_parser_h */
