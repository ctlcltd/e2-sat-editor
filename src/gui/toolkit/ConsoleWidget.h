/*!
 * e2-sat-editor/src/gui/toolkit/ConsoleWidget.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ConsoleWidget_h
#define ConsoleWidget_h
#include <QPlainTextEdit>
#include <QTextBlock>

namespace e2se_gui
{
class ConsoleWidget : public QPlainTextEdit
{
	Q_OBJECT

	public:
		explicit ConsoleWidget(QWidget* parent = nullptr);
		void prompt();
		void prompt(const QString cmd);
		void nav(const QString text);

		int gtpos = 0;

	signals:
		void input(Qt::Key key, const QString val);

	protected:
		void keyPressEvent(QKeyEvent* event) override;
};
}
#endif /* ConsoleWidget_h */
