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
#include <map>

using std::string, std::pair, std::vector, std::map;

#ifndef editService_h
#define editService_h
#include <QWidget>
#include <QComboBox>

#include "dialAbstract.h"

namespace e2se_gui
{
class editService : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editService)

	public:
		editService(dataHandler* data, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void serviceLayout();
		void transponderLayout();
		void paramsLayout();
		void tunerComboChanged(int index);
		void store();
		void retrieve();
		string getPIDValue(e2db::service ch, e2db::SDATA_PIDS x);
		vector<string> computePIDs(e2db::service ch, e2db::SDATA_PIDS x, string val);
		string getFlagValue(e2db::service ch, e2db::SDATA_FLAGS x);
		vector<string> computeFlags(e2db::service ch, e2db::SDATA_FLAGS x, string val);
		void setEditId(string chid);
		string getEditId();
		string getAddId();

		struct sts
		{
			// editable
			bool edit = false;
			// service data C (CAS) raw string
			string raw_C;
			// service data raw string
			string raw_data;
		} state;
	protected:
		QComboBox* dtf1tn;
		QComboBox* dtf1tx;
		map<string, vector<pair<int, string>>> txdata;
	private:
		string chid;
};
}
#endif /* editService_h */
