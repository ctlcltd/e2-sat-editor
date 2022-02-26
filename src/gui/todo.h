/*!
 * e2-sat-editor/src/gui/todo.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <string>

using namespace std;

#ifndef todo_h
#define todo_h
#include <QMessageBox>

namespace e2se_gui
{
static void todo()
{
	cout << "app TODO" << endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}
static void todoMsg(string msg)
{
	QMessageBox dial = QMessageBox();
	dial.setText(QString::fromStdString(msg));
	dial.exec();
}
}
#endif /* todo_h */
