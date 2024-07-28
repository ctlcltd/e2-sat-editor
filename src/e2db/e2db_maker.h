/*!
 * e2-sat-editor/src/e2db/e2db_maker.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../logger/logger.h"
#include "e2db_abstract.h"

#ifndef e2db_maker_h
#define e2db_maker_h
namespace e2se_e2db
{
class e2db_maker : virtual public e2db_abstract
{
	public:

		inline static bool MAKER_TUNERSETS = true;
		inline static bool MAKER_PARENTALLOCK_LIST = true;

		inline static string MAKER_SPATH = "/etc/enigma2";
		inline static string MAKER_TPATH = MAKER_SPATH;
		inline static string MAKER_BPATH = MAKER_SPATH;

		inline static bool MAKER_FIX_CRLF = true;

		e2db_maker();
		virtual ~e2db_maker() = default;
		void make_e2db();
		void make_e2db_lamedb();
		void make_e2db_lamedb(string filename, int ver);
		void make_e2db_bouquets();
		void make_e2db_bouquets(int ver);
		void make_e2db_userbouquets();
		void make_e2db_userbouquets(int ver);
		void make_db_tunersets();
		void make_zapit();
		void make_zapit_services();
		void make_zapit_services(int ver);
		void make_zapit_bouquets();
		void make_zapit_bouquets(int ver);
		void make_e2db_parentallock_list();
		void make_lamedb(string filename, e2db_file& file, int ver);
		void make_bouquet(string bname, e2db_file& file, int ver = 0);
		void make_bouquet_epl(string bname, e2db_file& file, int ver = 0);
		//TODO
		void make_userbouquet(string bname, e2db_file& file, bool exact_marker_index);
		void make_userbouquet(string bname, e2db_file& file);
		void make_tunersets_xml(string filename, int ytype, e2db_file& file);
		void make_services_xml(string filename, e2db_file& file, int ver);
		void make_bouquets_xml(string filename, e2db_file& file, int ver);
		void make_parentallock_list(string filename, PARENTALLOCK ltype, e2db_file& file);
		bool write(string path);
		unordered_map<string, e2db_file> get_output();

	protected:

		enum MAKER_FORMAT {
			b_comment = 0,
			b_transponders_start = 1,
			b_services_start = 2,
			b_section_end = 3,
			b_delimiter = 4,
			b_transponder_flag = 5,
			b_transponder_params_separator = 6,
			b_transponder_space_delimiter = 7,
			b_transponder_endline = 8,
			b_service_flag = 9,
			b_service_params_separator = 10,
			b_service_param_escape = 11,
			b_service_endline = 12
		};

		inline static const string LAMEDB5_FORMATS[13] = {"# ", "", "", "", ":", "t", ",", ":", "\n", "s", ",", "\"", "\n"};
		inline static const string LAMEDBX_FORMATS[13] = {"", "transponders\n", "services\n", "end\n", "", "", "\n\t", " ", "\n/\n", "", "\n", "", "\n"};

		bool push_file(string path);
		string conv_uri_value(string str);
		string conv_xml_value(string str);

		int marker_count = 0;
};
}
#endif /* e2db_maker_h */
