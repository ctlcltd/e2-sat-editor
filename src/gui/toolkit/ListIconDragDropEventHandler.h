/*!
 * e2-sat-editor/src/gui/toolkit/ListIconDragDropEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>

using std::string;

#ifndef ListIconDragDropEventHandler_h
#define ListIconDragDropEventHandler_h
#include <QObject>
#include <QEvent>
#include <QListWidget>

namespace e2se_gui
{
class ListIconDragDropEventHandler : public QObject
{
	public:
		ListIconDragDropEventHandler(QListWidget* source)
		{
			this->list = source;
		}
		void setEventCallback(std::function<void(QListWidgetItem* item, const QString path)> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventDragEnter(QObject* object, QEvent* event);
		bool eventDragMove(QObject* object, QEvent* event);
		bool eventDragLeave(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		void raiseWindow();
		void callEventCallback(QListWidgetItem* item, const QString path)
		{
			if (this->eventCallback)
				this->eventCallback(item, path);
		}

	private:
		QListWidget* list = nullptr;
		std::function<void(QListWidgetItem* item, const QString path)> eventCallback;
};

class ListIconDragDropEventFilter : public QObject
{
	protected:
		bool eventFilter(QObject* object, QEvent* event);
};
}
#endif /* ListIconDragDropEventHandler_h */
