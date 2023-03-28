/*!
 * e2-sat-editor/src/gui/toolkit/TreeDropIndicatorEventPainter.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef TreeDropIndicatorEventPainter_h
#define TreeDropIndicatorEventPainter_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class TreeDropIndicatorEventPainter : public QObject
{
	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventDragMove(QObject* object, QEvent* event);
		bool eventDragLeave(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		bool eventPaint(QObject* object, QEvent* event);

		QRect dropIndicatorRect;
};
}
#endif /* TreeDropIndicatorEventPainter_h */
