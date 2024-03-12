/*!
 * e2-sat-editor/src/gui/checkUpdate.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef checkUpdate_h
#define checkUpdate_h
#include <QApplication>
#include <QWidget>
#include <QMessageBox>

#include "../logger/logger.h"

namespace e2se_gui
{
class checkUpdate : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(checkUpdate)

	public:

		inline static bool VERBOSE = false;
		inline static int HTTP_CONNECT_TIMEOUT = 10;
		inline static int HTTP_TIMEOUT = 30;

		enum MEDIUM {
			unknown = 0,
			github = 1,
			snapstore = 2,
			flathub = 3,
			aur = 6
		};

		enum DIAL {
			dial_noupdate,
			dial_haveupdate,
			dial_connerror,
			dial_fetcherror
		};

		struct __state
		{
			DIAL dialog;
			bool checked = false;
			bool connected = false;
			bool fetched = false;
			string current_url;
			string latest_url;
			string version;
		};

		checkUpdate(QWidget* cwid);
		virtual ~checkUpdate();
		void check();
		void autoCheck();
		void destroy();

	protected:
		void show();
		void prompt();
		virtual void fetch();

		MEDIUM medium = MEDIUM::unknown;
		bool autocheck = false;

		__state state;

	private:
		static size_t data_write_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_discard_func(void* csi, size_t size, size_t nmemb, void* pso);

		QWidget* cwid;
};
}
#endif /* checkUpdate_h */
