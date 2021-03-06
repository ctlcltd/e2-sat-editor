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

		inline static const string LAMEDB4_FORMATS[13] = {"", "transponders\n", "services\n", "end\n", "", "", "\n\t", " ", "\n/\n", "", "\n", "", "\n"};
		inline static const string LAMEDB5_FORMATS[13] = {"# ", "", "", "", ":", "t", ",", ":", "\n", "s", ",", "\"", "\n"};

		// e2db_maker(e2se::logger::session log);
		// e2db_maker();
		virtual ~e2db_maker() = default;
		void make_e2db();
		string get_timestamp();
		string get_editor_string();
		void make_e2db_lamedb();
		void make_e2db_lamedb4();
		void make_e2db_lamedb5();
		void make_e2db_bouquets();
		void make_e2db_userbouquets();
		void make_db_tunersets();
		void set_index(unordered_map<string, vector<pair<int, string>>> index);
		void set_transponders(unordered_map<string, transponder> transponders);
		void set_channels(unordered_map<string, service> services);
		void set_bouquets(pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> bouquets);
		bool write_to_localdir(string localdir, bool overwrite);
		bool write(string localdir, bool overwrite);
		unordered_map<string, e2db_file> get_output();
	protected:
		e2db_file make_lamedb(string filename);
		e2db_file make_lamedb4(string filename);
		e2db_file make_lamedb5(string filename);
		e2db_file make_bouquet(string bname);
		e2db_file make_userbouquet(string bname);
		e2db_file make_tunersets_xml(string filename, int ytype);
	private:
		// e2db_out <filename string, e2db_file>
		unordered_map<string, e2db_file> e2db_out;
};
}
#endif /* e2db_maker_h */
