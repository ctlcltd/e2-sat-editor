/*!
 * e2-sat-editor/src/gui/editService.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
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
#include <QTabWidget>
#include <QComboBox>

#include "dialAbstract.h"

namespace e2se_gui
{
class editTransponder;
class editFavourite;

class editService : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editService)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// service edit
			bool service = false;
			// transponder edit
			bool transponder = false;
			// favourite edit
			bool favourite = false;
			// is fav
			bool is_fav = false;
			// is stream
			bool is_stream = false;
			// service data C (CAID) raw string
			string raw_C;
			// service data raw string
			string raw_data;
		};

		editService(dataHandler* data);
		~editService();
		void display(QWidget* cwid);
		bool destroy();
		void setEditId(string chid);
		void setEditId(string chid, string bname, bool stream = false);
		string getEditId();
		void setAddId(string bname);
		string getTransponderId();
		bool isFavourite();
		bool isStream();
		bool getReload();

	protected:
		void layout(QWidget* cwid);
		void serviceLayout();
		void transponderLayout();
		void paramsLayout();
		void tunerComboChanged(int index);
		void transponderComboChanged(int index);
		void tabChanged(int index);
		void newTransponder();
		void store();
		void retrieve();
		string getPIDValue(e2db::service ch, e2db::SDATA_PIDS x);
		vector<string> computePIDs(e2db::service ch, e2db::SDATA_PIDS x, string val);
		string getFlagValue(e2db::service ch, e2db::SDATA_FLAGS x);
		vector<string> computeFlags(e2db::service ch, e2db::SDATA_FLAGS x, string val);
		bool checkCollision(e2db::service ch);

		QTabWidget* dtwid;
		QGridLayout* dtpage;
		QComboBox* dtf1tn;
		QComboBox* dtf1tx;
		editTransponder* edittxp = nullptr;
		editFavourite* editfav = nullptr;
		map<string, vector<pair<int, string>>> txp_index;

		__state state;
		__action action;

	private:
		string chid;
		string txid;
		string bname;
		string ref_txid;
		string ref_chid;
};
}
#endif /* editService_h */
