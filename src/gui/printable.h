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
		enum view {
			index=-1,
			services=0,
			bouquets=1,
			userbouquets=2,
			tunersets=3
		};

		struct page
		{
			QString header;
			QString body;
			QString footer;
		};

		printable(e2db* dbih, e2se::logger::session* log);
		void document_all();
		void document_index();
		void document_lamedb();
		void document_lamedb(int stype);
		void document_bouquet(string bname);
		void document_userbouquet(string bname);
		void document_tunersets(int ytype);
		void print();
		void destroy() {};
		QSettings* sets;
	protected:
		void page_header(page& p, string filename, view v);
		void page_footer(page& p, string filename, view v);
		void page_body_index_list(page& p, vector<string> paths);
		void page_body_channel_list(page& p, string bname, view v);
		void page_body_bouquet_list(page& p, string bname);
		void page_body_tunersets_list(page& p, int ytype);
		QString doc_head();
		QString doc_foot();
	private:
		e2db* dbih;
		vector<page> pages;
};
}
#endif /* printable_h */
