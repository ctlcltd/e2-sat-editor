/*!
 * e2-sat-editor/src/e2db/e2db_converter.h
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

#ifndef e2db_converter_h
#define e2db_converter_h
namespace e2se_e2db
{
class e2db_converter : virtual public e2db_abstract
{
	public
		// e2db_converter(e2se::logger::session log);
		// e2db_converter();
		virtual ~e2db_converter() = default;
		void from_file(vector<string> paths);
		// void from_file(FPORTS fpi, e2db* dbih, e2db_file file, string path);
		void to_file(vector<string> paths);
		// void to_file(FPORTS fpo, vector<string> paths);
		// void to_file(FPORTS fpo, string path);
		void from_csv_lamedb(istream& ifile);
		void from_csv_bouquet(istream& ifile, string bname);
		void from_csv_userbouquet(istream& ifile, string bname);
		void from_csv_tunersets(int ytype, istream& ifile);
		e2db_file to_csv_lamedb(string filename);
		e2db_file to_csv_bouquet(string bname);
		e2db_file to_csv_userbouquet(string bname);
		e2db_file to_csv_tunersets(string filename, int ytype);
		e2db_file to_html_lamedb(string filename);
		e2db_file to_html_bouquet(string bname);
		e2db_file to_html_userbouquet(string bname);
		e2db_file to_html_tunersets(string filename, int ytype);
		// bool read(string localdir);
		// bool write_to_localdir(string localdir, bool overwrite);
		// bool write(string localdir, bool overwrite);
};
}
#endif /* e2db_converter_h */
