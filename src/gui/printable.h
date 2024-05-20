/*!
 * e2-sat-editor/src/gui/printable.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <map>

using std::string, std::pair, std::vector, std::map;

#ifndef printable_h
#define printable_h
#include <QApplication>
#include <QWidget>

#include "../logger/logger.h"
#include "dataHandler.h"

namespace e2se_gui
{
class printable : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(printable)

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

		printable(QWidget* cwid, dataHandler* data);
		virtual ~printable();
		void documentAll();
		void documentIndex();
		void documentServices();
		void documentServices(int stype);
		void documentBouquet(string bname);
		void documentUserbouquet(string bname);
		void documentTunersets(int ytype);
		void print();

	protected:
		void pageHeader(html_page& page, string filename, DOC_VIEW view);
		void pageFooter(html_page& page, string filename, DOC_VIEW view);
		void pageBodyIndexList(html_page& page, vector<string> paths);
		void pageBodyChannelList(html_page& page, string bname, DOC_VIEW view);
		void pageBodyBouquetList(html_page& page, string bname);
		void pageBodyTunersetsList(html_page& page, int ytype);
		QString docHtmlHead();
		QString docHtmlFoot();

		dataHandler* data = nullptr;

	private:
		QWidget* cwid;
		vector<html_page> pages;
};
}
#endif /* printable_h */
