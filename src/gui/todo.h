/*!
 * e2-sat-editor/src/gui/todo.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <QMessageBox>

using namespace std;

#ifndef todo_h
#define todo_h

namespace e2se_gui
{
static void todo()
{
	cout << "app TODO" << endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}
}
#endif /* todo_h */
