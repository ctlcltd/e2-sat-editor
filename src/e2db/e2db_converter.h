/*!
 * e2-sat-editor/src/e2db/e2db_converter.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <fstream>

#include "../logger/logger.h"
#include "e2db_abstract.h"

using std::istream, std::string;

#ifndef e2db_converter_h
#define e2db_converter_h
namespace e2se_e2db
{
class e2db;

//TODO current tunersets
class e2db_converter : virtual public e2db_abstract
{
	public:

		inline static bool CONVERTER_EXTENDED_FIELDS = false;
		inline static bool CSV_HEADER = true;
		inline static string CSV_DELIMITER = "\n";
		inline static char CSV_SEPARATOR = ',';
		inline static char CSV_ESCAPE = '"';

		//TODO CRLF
		// inline static string M3U_DELIMITER = "\r\n";
		inline static string M3U_DELIMITER = "\n";

		enum FCONVS {
			convert_current = 0x0,
			convert_all = 0x2,
			convert_index = 0x1,
			convert_services = 0x10,
			convert_bouquets = 0x20,
			convert_userbouquets = 0x40,
			convert_tunersets = 0x80
		};

		enum DOC_VIEW {
			view_index = -1,
			view_services = 0,
			view_bouquets = 1,
			view_userbouquets = 2,
			view_tunersets = 3
		};

		enum M3U_FLAGS {
			m3u_singular = 0x10,
			m3u_chrefid = 0x1,
			m3u_chnum = 0x2,
			m3u_chgroup = 0x4,
			m3u_chlogos = 0x8,
			m3u_default = 0xf
		};

		struct fcopts
		{
			FCONVS fc;
			string filename;
			string bname;
			int stype;
			int ytype;
			int flags;
			string logosbase;
		};

		struct m3u_entry
		{
			channel_reference chref;
			string ub_name;
			string ch_logo;
			int ch_num = 0;
		};

		struct html_page
		{
			string title;
			string header;
			string body;
			string footer;
		};

		e2db_converter();
		virtual ~e2db_converter() = default;
		void import_csv_file(FCONVS fci, fcopts opts, vector<string> paths);
		void import_csv_file(FCONVS fci, fcopts opts, string path);
		void import_csv_file(FCONVS fci, fcopts opts, e2db_abstract* dst, string path);
		void export_csv_file(FCONVS fco, fcopts opts, string path);
		void export_html_file(FCONVS fco, fcopts opts, string path);
		void import_m3u_file(FCONVS fci, fcopts opts, vector<string> paths);
		void import_m3u_file(FCONVS fci, fcopts opts, string path);
		void import_m3u_file(FCONVS fci, fcopts opts, e2db_abstract* dst, string path);
		void export_m3u_file(FCONVS fco, fcopts opts, vector<string> ubouquets, string path);
		void pull_csv_services(istream& ifile, e2db_abstract* dst);
		void pull_csv_bouquets(istream& ifile, e2db_abstract* dst);
		void pull_csv_userbouquets(istream& ifile, e2db_abstract* dst);
		void pull_csv_tunersets(istream& ifile, e2db_abstract* dst);
		void push_csv_all(vector<e2db_file>& files);
		void push_csv_services(vector<e2db_file>& files);
		void push_csv_services(vector<e2db_file>& files, int stype);
		void push_csv_bouquets(vector<e2db_file>& files);
		void push_csv_bouquets(vector<e2db_file>& files, string bname, string filename);
		void push_csv_userbouquets(vector<e2db_file>& files);
		void push_csv_userbouquets(vector<e2db_file>& files, string bname, string filename);
		void push_csv_tunersets(vector<e2db_file>& files);
		void push_csv_tunersets(vector<e2db_file>& files, int ytype);
		void pull_m3u_list(istream& ifile, e2db_abstract* dst, fcopts opts);
		void push_m3u_list(vector<e2db_file>& files, vector<string> ubouquets, fcopts opts);
		void push_m3u_list(vector<e2db_file>& files, string bname, fcopts opts);
		void push_html_all(vector<e2db_file>& files);
		void push_html_index(vector<e2db_file>& files);
		void push_html_services(vector<e2db_file>& files);
		void push_html_services(vector<e2db_file>& files, int stype);
		void push_html_bouquets(vector<e2db_file>& files);
		void push_html_bouquets(vector<e2db_file>& files, string bname, string filename);
		void push_html_userbouquets(vector<e2db_file>& files);
		void push_html_userbouquets(vector<e2db_file>& files, string bname, string filename);
		void push_html_tunersets(vector<e2db_file>& files);
		void push_html_tunersets(vector<e2db_file>& files, int ytype);

	protected:
		virtual e2db_converter* newptr() { return new e2db_converter; }
		void parse_csv(istream& ifile, vector<vector<string>>& sxv);
		void parse_m3u(istream& ifile, unordered_map<string, vector<m3u_entry>>& cxm);
		void convert_csv_channel_list(vector<vector<string>>& sxv, e2db_abstract* dst, DOC_VIEW view);
		void convert_csv_channel_list_extended(vector<vector<string>>& sxv, e2db_abstract* dst, DOC_VIEW view);
		void convert_csv_bouquet_list(vector<vector<string>>& sxv, e2db_abstract* dst);
		void convert_csv_tunersets_list(vector<vector<string>>& sxv, e2db_abstract* dst);
		void csv_channel_list(string& csv, string bname, DOC_VIEW view);
		void csv_channel_list_extended(string& csv, string bname, DOC_VIEW view);
		void csv_bouquet_list(string& csv, string bname);
		void csv_tunersets_list(string& csv, int ytype);
		void convert_m3u_channel_list(unordered_map<string, vector<m3u_entry>>& cxm, e2db_abstract* dst, fcopts opts);
		void m3u_channel_list(string& body, string bname, fcopts opts);
		void page_header(html_page& page, string filename, DOC_VIEW view);
		void page_footer(html_page& page, string filename, DOC_VIEW view);
		void page_body_index_list(html_page& page, vector<string> paths);
		void page_body_channel_list(html_page& page, string bname, DOC_VIEW view);
		void page_body_bouquet_list(html_page& page, string bname);
		void page_body_tunersets_list(html_page& page, int ytype);
		void csv_document(e2db_file& file, string csv);
		void m3u_document(e2db_file& file, string body);
		void html_document(e2db_file& file, html_page page);
		string filename_format(string fname, string fext);
		string doc_html_head(html_page page);
		string doc_html_foot(html_page page);
		string conv_picon_pathname(string str);
};
}
#endif /* e2db_converter_h */
