/*!
 * e2-sat-editor/src/gui/toolkit/WidgetWithBackdrop.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef WidgetWithBackdrop_h
#define WidgetWithBackdrop_h
#include <QWidget>

namespace e2se_gui
{
class WidgetWithBackdrop : public QWidget
{
	Q_OBJECT

	public:
		explicit WidgetWithBackdrop(QWidget* parent = nullptr);

	signals:
		void backdrop();

	protected:
		void mousePressEvent(QMouseEvent* event)
		{
			emit backdrop();

			return QWidget::mousePressEvent(event);
		}
};
}
#endif /* WidgetWithBackdrop_h */
