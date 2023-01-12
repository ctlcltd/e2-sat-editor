/*!
 * e2-sat-editor/src/gui/editService.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
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
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// service data C (CAID) raw string
			string raw_C;
			// service data raw string
			string raw_data;
		};

		editService(dataHandler* data, e2se::logger::session* log);
		void display(QWidget* cwid);
		void setEditId(string chid);
		string getEditId();
		string getAddId();

	protected:
		void layout(QWidget* cwid);
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

		QComboBox* dtf1tn;
		QComboBox* dtf1tx;
		map<string, vector<pair<int, string>>> txdata;

		__state state;
		__action action;

	private:
		string chid;
		string txid;
};
}
#endif /* editService_h */
