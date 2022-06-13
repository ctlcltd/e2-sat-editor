/*!
 * e2-sat-editor/src/gui/ListEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ListEventHandler_h
#define ListEventHandler_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class ListEventHandler : public QObject
{
    public:
        void disallowInternalMove()
        {
            this->dnd = false;
        }
        void allowInternalMove()
        {
            this->dnd = true;
        }
	protected:
		bool eventFilter(QObject* o, QEvent* e);
	private:
		bool dnd = true;
};
}
#endif /* ListEventHandler_h */
