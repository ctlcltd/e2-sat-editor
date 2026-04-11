/*!
 * e2-sat-editor/src/gui/toolkit/DialogDockWidget.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QWindow>
#include <QDebug>

#include "DialogDockWidget.h"

namespace e2se_gui
{

DialogDockWidget::DialogDockWidget(QWidget* parent, Qt::WindowFlags flags) : QDockWidget(parent, flags)
{
	this->setContextMenuPolicy(Qt::PreventContextMenu);

	connect(this, &DialogDockWidget::dockLocationChanged, this, [=](Qt::DockWidgetArea area) {
		this->docked = (area != Qt::NoDockWidgetArea);
	});
}

void DialogDockWidget::setLayout(QLayout* layout)
{
	QWidget* wid = new QWidget;
	wid->setLayout(layout);
	this->setWidget(wid);
}

void DialogDockWidget::setWidgetParent(QWidget* parent)
{
	if (! this->docked && this->isFloating())
	{
		qDebug() << this->objectName() << " floating setWidgetParent";

		this->blockSignals(true);
		QRect geometry = this->geometry();
		this->setParent(parent);
		this->setFloating(true);
		this->setGeometry(geometry);
		this->show();
		this->blockSignals(false);
	}
	else
	{
		qDebug() << this->objectName() << " not floating setWidgetParent";
		this->setParent(parent);
		this->show();
	}
}

void DialogDockWidget::raiseWindow()
{
	if (! this->docked && this->isFloating())
	{
		qDebug() << this->objectName() << " floating raise";
		qDebug() << this->objectName() << " " << this->dockLocation();
		this->windowHandle()->requestActivate();
		this->raise();
	}
}

bool DialogDockWidget::isDocked()
{
	qDebug() << this->objectName() << " isDocked: " << this->docked;
	return this->docked;
}

}
