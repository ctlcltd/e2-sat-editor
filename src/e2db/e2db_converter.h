/*!
 * e2-sat-editor/src/e2db/e2db_converter.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
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

class e2db_converter : virtual public e2db_abstract
{
	public:
		enum FCONVS {
			convert_current = 0x0,
			convert_all = 0x2,
			convert_index = 0x1,
			convert_services = 0x10,
			convert_bouquets = 0x20,
			convert_userbouquets = 0x40,
			convert_tunersets = 0x80 //TODO current
		};

		enum DOC_VIEW {
			view_index = -1,
			view_services = 0,
			view_bouquets = 1,
			view_userbouquets = 2,
			view_tunersets = 3
		};

		inline static bool CONVERTER_EXTENDED_FIELDS = false;
		inline static bool CSV_HEADER = true;
		inline static char CSV_DELIMITER = '\n';
		inline static char CSV_SEPARATOR = ',';
		inline static char CSV_ESCAPE = '"';

		struct fcopts
		{
			FCONVS fc;
			string filename;
			string bname;
			int stype;
			int btype;
			int ytype;
		};

		struct html_page
		{
			string title;
			string header;
			string body;
			string footer;
		};

		explicit e2db_converter();
		e2db_converter(e2se::logger::session* log);
		virtual ~e2db_converter() = default;
		virtual e2db_converter* newptr() { return new e2db_converter(this->log->log); }
		void import_csv_file(FCONVS fci, fcopts opts, vector<string> paths);
		void import_csv_file(FCONVS fci, fcopts opts, string path);
		void import_csv_file(FCONVS fci, fcopts opts, e2db_abstract* dst, string path);
		void export_csv_file(FCONVS fco, fcopts opts, string path);
		void export_html_file(FCONVS fco, fcopts opts, string path);
		void pull_csv_services(istream& ifile, e2db_abstract* dst);
		void pull_csv_bouquets(istream& ifile, e2db_abstract* dst);
		void pull_csv_userbouquets(istream& ifile, e2db_abstract* dst);
		void pull_csv_tunersets(istream& ifile, e2db_abstract* dst);
		void push_csv_all(vector<e2db_file>& files);
		void push_csv_services(vector<e2db_file>& files);
		void push_csv_services(vector<e2db_file>& files, int stype);
		void push_csv_bouquets(vector<e2db_file>& files);
		void push_csv_bouquets(vector<e2db_file>& files, string bname);
		void push_csv_userbouquets(vector<e2db_file>& files);
		void push_csv_userbouquets(vector<e2db_file>& files, string bname);
		void push_csv_tunersets(vector<e2db_file>& files);
		void push_csv_tunersets(vector<e2db_file>& files, int ytype);
		void push_html_all(vector<e2db_file>& files);
		void push_html_index(vector<e2db_file>& files);
		void push_html_services(vector<e2db_file>& files);
		void push_html_services(vector<e2db_file>& files, int stype);
		void push_html_bouquets(vector<e2db_file>& files);
		void push_html_bouquets(vector<e2db_file>& files, string bname);
		void push_html_userbouquets(vector<e2db_file>& files);
		void push_html_userbouquets(vector<e2db_file>& files, string bname);
		void push_html_tunersets(vector<e2db_file>& files);
		void push_html_tunersets(vector<e2db_file>& files, int ytype);
	protected:
		void parse_csv(istream& ifile, vector<vector<string>>& sxv);
		void convert_csv_channel_list(vector<vector<string>> sxv, e2db_abstract* dst, DOC_VIEW view);
		void convert_csv_channel_list_extended(vector<vector<string>> sxv, e2db_abstract* dst, DOC_VIEW view);
		void convert_csv_bouquet_list(vector<vector<string>> sxv, e2db_abstract* dst);
		void convert_csv_tunersets_list(vector<vector<string>> sxv, e2db_abstract* dst);
		void csv_channel_list(string& csv, string bname, DOC_VIEW view);
		void csv_channel_list_extended(string& csv, string bname, DOC_VIEW view);
		void csv_bouquet_list(string& csv, string bname);
		void csv_tunersets_list(string& csv, int ytype);
		void page_header(html_page& page, string filename, DOC_VIEW view);
		void page_footer(html_page& page, string filename, DOC_VIEW view);
		void page_body_index_list(html_page& page, vector<string> paths);
		void page_body_channel_list(html_page& page, string bname, DOC_VIEW view);
		void page_body_bouquet_list(html_page& page, string bname);
		void page_body_tunersets_list(html_page& page, int ytype);
		void html_document(e2db_file& file, html_page page);
		void csv_document(e2db_file& file, string csv);
		string doc_html_head(html_page page);
		string doc_html_foot(html_page page);
};
}
#endif /* e2db_converter_h */
