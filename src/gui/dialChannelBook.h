/*!
 * e2-sat-editor/src/gui/dialChannelBook.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <vector>

using std::vector;

#ifndef dialChannelBook_h
#define dialChannelBook_h
#include <QWidget>
#include <QCheckBox>

#include "dialAbstract.h"
#include "channelBookView.h"

namespace e2se_gui
{
class dialChannelBook : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(dialChannelBook)

	public:
		struct __action
		{
			QAction* cancel;
			QAction* submit;
			QCheckBox* filter;
		};

		dialChannelBook(dataHandler* data, int stype);
		~dialChannelBook();
		void display(QWidget* cwid);
		void load();
		void reset();
		void reload();
		bool destroy();
		void setEventCallback(std::function<void(vector<QString> items)> func)
		{
			this->eventCallback = func;
		}

	protected:
		void layout(QWidget* cwid);
		void toolbarLayout();
		void store() {}
		void retrieve() {}
		void submit();
		void callEventCallback(vector<QString> items)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(items);
		}

		int stype;

		__state state;
		__action action;

	private:
		channelBookView* cbv = nullptr;
		std::function<void(vector<QString> items)> eventCallback;
};
}
#endif /* dialChannelBook_h */
