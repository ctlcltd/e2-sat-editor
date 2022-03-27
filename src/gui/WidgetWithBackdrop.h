/*!
 * e2-sat-editor/src/gui/WidgetWithBackdrop.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
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
		void activateBackdrop()
		{
			enabled = true;
		}
		void deactivateBackdrop()
		{
			enabled = false;
		}
	signals:
		void backdrop();
	protected:
		void mousePressEvent(QMouseEvent* e)
		{
			if (enabled) emit backdrop();
		}
	private:
		bool enabled = false;
};
}
#endif /* WidgetWithBackdrop_h */
