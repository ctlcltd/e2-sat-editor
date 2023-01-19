/*!
 * e2-sat-editor/src/gui/todo.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>

#ifndef todo_h
#define todo_h
#include <QMessageBox>

namespace e2se_gui
{
static void todo()
{
	std::cout << "app TODO" << std::endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}
}
#endif /* todo_h */
