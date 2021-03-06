/*!
 * e2-sat-editor/src/gui/editService.h
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

using std::string, std::pair, std::vector;

#ifndef editService_h
#define editService_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QComboBox>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class editService : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(editService)

	public:
		editService(e2db* dbih, e2se::logger::session* log);
		void display(QWidget* cwid);
		void serviceLayout();
		void transponderLayout();
		void paramsLayout();
		void tunerComboChanged(int index);
		void store();
		void retrieve();
		void save();
		string getPIDValue(e2db::service ch, e2db::SDATA_PIDS x);
		vector<string> computePIDs(e2db::service ch, e2db::SDATA_PIDS x, string val);
		string getFlagValue(e2db::service ch, e2db::SDATA_FLAGS x);
		vector<string> computeFlags(e2db::service ch, e2db::SDATA_FLAGS x, string val);
		void setEditID(string chid);
		string getEditID();
		void destroy();
		QWidget* widget;
	protected:
		QDialog* dial;
		QGridLayout* dtform;
		QComboBox* dtf1tn;
		QComboBox* dtf1tx;
		map<string, vector<pair<int, string>>> txdata;
		vector<QWidget*> fields;
	private:
		struct sts
		{
			// editable
			bool edit = false;
			// service data C (CAS) raw string
			string raw_C;
			// service data raw string
			string raw_data;
		} state;
		e2db* dbih;
		string chid;
};
}
#endif /* editService_h */
