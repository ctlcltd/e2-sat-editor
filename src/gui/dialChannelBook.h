/*!
 * e2-sat-editor/src/gui/dialChannelBook.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <vector>

using std::vector;

#ifndef dialChannelBook_h
#define dialChannelBook_h
#include <QWidget>

#include "dialAbstract.h"
#include "channelBookView.h"

namespace e2se_gui
{
class dialChannelBook : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(dialChannelBook)

	public:
		dialChannelBook(dataHandler* data, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void toolbar();
		void store() {};
		void retrieve() {};
		void sender();
		void setEventCallback(std::function<void(vector<QString> items)> func)
		{
			this->eventCallback = func;
		}

		struct __action
		{
			QAction* add;
		} action;
	protected:
		void callEventCallback(vector<QString> items)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(items);
		}
	private:
		channelBookView* cbv = nullptr;
		std::function<void(vector<QString> items)> eventCallback;
};
}
#endif /* dialChannelBook_h */
