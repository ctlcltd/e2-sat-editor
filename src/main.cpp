/*!
 * e2-sat-editor/src/main.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include "gui_qt6.h"

int main(int argc, char* argv[])
{
	(new gui())->root(argc, argv);

	return 0;
}
