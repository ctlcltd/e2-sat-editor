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
		enum HANDLE {
			Command,
			Listing,
			Input
		};

		explicit ConsoleWidget(QWidget* parent = nullptr);
		void attach(QWidget* parent);
		void detach();
		void output(const QString text);
		void error(const QString text);
		void prompt();
		void nav();
		void beep();
		void ruler();

	signals:
		void input(const int key, const QString val);

	protected:
		void maybeInsertBlock(QTextCursor &cursor);
		void keyPressEvent(QKeyEvent* event) override;
		bool canInsertFromMimeData(const QMimeData* source) const override;
		void insertFromMimeData(const QMimeData* source) override;
		void showContextMenu(QPoint pos);

	private:
		HANDLE currhr;
		int gtpos = 0;
};
}
#endif /* ConsoleWidget_h */
