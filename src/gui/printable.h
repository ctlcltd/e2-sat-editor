/*!
 * e2-sat-editor/src/gui/printable.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <map>

using std::string, std::pair, std::vector, std::map;

#ifndef printable_h
#define printable_h

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class printable : protected e2se::log_factory
{
	public:
		// relation with tab state tc (side tree focused)
		enum DOC_VIEW {
			view_index = -1,
			view_services = 0,
			view_bouquets = 1,
			view_userbouquets = 2,
			view_tunersets = 3
		};

		struct html_page
		{
			QString header;
			QString body;
			QString footer;
		};

		printable(e2db* dbih, e2se::logger::session* log);
		void document_all();
		void document_index();
		void document_services();
		void document_services(int stype);
		void document_bouquet(string bname);
		void document_userbouquet(string bname);
		void document_tunersets(int ytype);
		void print();
		void destroy() {};
		QSettings* sets;
	protected:
		void page_header(html_page& page, string filename, DOC_VIEW view);
		void page_footer(html_page& page, string filename, DOC_VIEW view);
		void page_body_index_list(html_page& page, vector<string> paths);
		void page_body_channel_list(html_page& page, string bname, DOC_VIEW view);
		void page_body_bouquet_list(html_page& page, string bname);
		void page_body_tunersets_list(html_page& page, int ytype);
		QString doc_html_head();
		QString doc_html_foot();
	private:
		e2db* dbih;
		vector<html_page> pages;
};
}
#endif /* printable_h */
