/*!
 * e2-sat-editor/src/gui/toolkit/WidgetBackdrop.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef WidgetBackdrop_h
#define WidgetBackdrop_h
#include <QWidget>

namespace e2se_gui
{
class WidgetBackdrop : public QWidget
{
	Q_OBJECT

	public:
		explicit WidgetBackdrop(QWidget* parent = nullptr);

	signals:
		void backdropEntered();

	protected:
		void mousePressEvent(QMouseEvent* event)
		{
			emit backdropEntered();

			return QWidget::mousePressEvent(event);
		}
};
}
#endif /* WidgetBackdrop_h */
