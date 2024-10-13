/*!
 * e2-sat-editor/src/gui/ftpHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ftpHandler_h
#define ftpHandler_h

#include <QtGlobal>

#include "../logger/logger.h"
#ifdef Q_OS_WASM
#include "ftpcom_gui_wasm.h"
#else
#include "ftpcom_gui.h"
#endif

namespace e2se_gui
{
class ftpHandler : protected e2se::log_factory
{
	public:
		ftpHandler();
		virtual ~ftpHandler();
		bool openConnection();
		bool closeConnection();
		bool handleConnection();
		bool isConnected();
		bool isDisconnected();
		void settingsChanged();
		string getServerHostname();

		ftpcom* ftih = nullptr;

	protected:
		bool connected = false;
};
}
#endif /* ftpHandler_h */
