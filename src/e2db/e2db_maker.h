/*!
 * e2-sat-editor/src/e2db/e2db_maker.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
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

		inline static bool MAKER_LAMEDB5 = false;
		inline static bool MAKER_TUNERSETS = true;

		enum MAKER_FORMAT {
			comment = 0,
			transponders_start = 1,
			services_start = 2,
			section_end = 3,
			delimiter = 4,
			transponder_flag = 5,
			transponder_params_separator = 6,
			transponder_space_delimiter = 7,
			transponder_endline = 8,
			service_flag = 9,
			service_params_separator = 10,
			service_param_escape = 11,
			service_endline = 12
		};
		inline static const string LAMEDB4_FORMATS[13] = {"", "transponders\n", "services\n", "end\n", "", "", "\n\t", " ", "\n/\n", "", "\n", "", "\n"};
		inline static const string LAMEDB5_FORMATS[13] = {"# ", "", "", "", ":", "t", ",", ":", "\n", "s", ",", "\"", "\n"};

		explicit e2db_maker();
		e2db_maker(e2se::logger::session* log);
		virtual ~e2db_maker() = default;
		void make_e2db();
		void make_e2db_lamedb();
		void make_e2db_lamedb4();
		void make_e2db_lamedb5();
		void make_e2db_bouquets();
		void make_e2db_userbouquets();
		void make_db_tunersets();
		bool push_file(string path);
		bool write(string path);
		unordered_map<string, e2db_file> get_output();
	protected:
		void make_lamedb(string filename, e2db_file& file);
		void make_lamedb4(string filename, e2db_file& file);
		void make_lamedb5(string filename, e2db_file& file);
		void make_bouquet(string bname, e2db_file& file);
		void make_userbouquet(string bname, e2db_file& file);
		void make_tunersets_xml(string filename, int ytype, e2db_file& file);
};
}
#endif /* e2db_maker_h */
