/*!
 * e2-sat-editor/src/gui/ftpHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ftpHandler_h
#define ftpHandler_h

#include "../logger/logger.h"
#include "ftpcom_gui.h"

namespace e2se_gui
{
class ftpHandler : protected e2se::log_factory
{
	public:
		ftpHandler(e2se::logger::session* log);
		~ftpHandler();
		bool openConnection();
		bool closeConnection();
		bool handleConnection();
		void settingsChanged();

		ftpcom* ftih = nullptr;
};
}
#endif /* ftpHandler_h */
