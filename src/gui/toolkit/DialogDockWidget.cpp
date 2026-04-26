/*!
 * e2-sat-editor/src/gui/toolkit/DialogDockWidget.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QWindow>
#include <QDebug>
#include <QTimer>

#include "DialogDockWidget.h"

namespace e2se_gui
{

DialogDockWidget::DialogDockWidget(QWidget* parent, Qt::WindowFlags flags) : QDockWidget(parent, flags)
{
	this->setContextMenuPolicy(Qt::PreventContextMenu);

	connect(this, &DialogDockWidget::topLevelChanged, this, [=](bool topLevel) {
		this->docked = ! topLevel;
	});
}

void DialogDockWidget::setLayout(QLayout* layout)
{
	QWidget* wid = new QWidget;
	wid->setLayout(layout);
	this->setWidget(wid);
}

void DialogDockWidget::raiseWindow()
{
	if (! this->docked && this->isFloating())
	{
		this->windowHandle()->requestActivate();
		this->raise();
	}
}

bool DialogDockWidget::isDocked()
{
	return this->docked;
}

}
