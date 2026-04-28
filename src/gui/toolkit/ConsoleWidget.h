/*!
 * e2-sat-editor/src/gui/toolkit/ConsoleWidget.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
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
		void attachWidget();
		void detachWidget();
		void printOutput(const QString text);
		void printErrors(const QString text);
		void printHistory(const Qt::Key key, const QString text);
		void printPromptCursor();
		void printNavigationRuler();
		void printSessionRuler();

		HANDLE currentHandler() const;
		void setCurrentHandler(HANDLE handle);
		void setCurrentHandler(int handle);
		bool isInputMasked() const;
		void setInputMasked(bool masked);
		void reset();

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
		int tcpos = 0;
		int tspos = 0;
		int impos = 0;
		bool imval = false;
		int nbpos = 0;
		int nblen = 0;
};
}
#endif /* ConsoleWidget_h */
