/*!
 * e2-sat-editor/src/gui/toolkit/DialogDockWidget.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef DialogDockWidget_h
#define DialogDockWidget_h
#include <QDockWidget>

namespace e2se_gui
{
class DialogDockWidget : public QDockWidget
{
	Q_OBJECT

	public:
		explicit DialogDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
		void setLayout(QLayout* layout);
		void setWidgetParent(QWidget* widget);
		void raiseWindow();
		bool isDocked();

	signals:
		void finished(int result);

	public slots:
		virtual void open() { this->show(); }
		virtual int exec() { this->show(); return 1; }

	protected:
		bool docked = true; // initial

		void closeEvent(QCloseEvent* event)
		{
			emit finished(1);
		}
};
}
#endif /* DialogDockWidget_h */
