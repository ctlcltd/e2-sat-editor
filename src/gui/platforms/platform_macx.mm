/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#import <AppKit/AppKit.h>
#include <iostream>
#include <QtGui>
#include <QProxyStyle>
#include <QTimer>
#include <QGridLayout>
#include "platform_macx.h"

class _ComboBoxProxyStyle : public QProxyStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const override
		{
			if (hint == QStyle::SH_ComboBox_UseNativePopup)
				return 1;
			
			return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};


QWidget* _platform_macx::_osWindowBlend(QWidget* widget, FX_MATERIAL material) {
	if (! NSClassFromString(@"NSVisualEffectView"))
		return widget;

	// QWindow* topLevelWindow = QGuiApplication::topLevelWindows().first();

	widget->setAttribute(Qt::WA_TranslucentBackground);

	NSView* view = (NSView*)widget->winId();
	NSView* superview = view.superview;
	NSVisualEffectView* subview = (NSVisualEffectView*)superview.subviews.firstObject;

	[subview setMaterial:NSVisualEffectMaterial (material)];
	[subview setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
	[subview setWantsLayer:FALSE];
	
	return widget;
};

//WONTFIX performance issues
// use _platform_macx::osWindowBlend instead
//
// huge impact (~400MB RAM with main window maximized)
// native widgets and backing store
// see QWidget documentation for more details
// section: Native Widgets vs Alien Widgets
//
QWidget* _platform_macx::_osWidgetBlend(QWidget* widget, FX_MATERIAL material)
{
	// if (! NSClassFromString(@"NSVisualEffectView"))
	// 	return widget;
	//
	// // const NSRect frameRect = {
	// // 	{0.0, 0.0},
	// // 	{static_cast<double>(widget->width()), static_cast<double>(widget->height())}
	// // };
	// // NSVisualEffectView* subview = [[[NSVisualEffectView alloc] initWithFrame:frameRect] autorelease];
	//
	// NSVisualEffectView* subview = [[NSVisualEffectView alloc] init];
	//
	// [subview setMaterial:NSVisualEffectMaterial (material)];
	// [subview setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
	// // [subview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	// [subview setWantsLayer:FALSE];
	// // [view setAutoresizesSubviews:YES];
	//
	// QWindow* qSubview = QWindow::fromWinId(reinterpret_cast<WId>(subview));
	// QWidget* blended = QWidget::createWindowContainer(qSubview);
	// // force stack under
	// blended->winId(); // transforms the whole stack to native QNSWindow / QNSView
	// blended->setAttribute(Qt::WA_PaintOnScreen, false);
	// // blended->setUpdatesEnabled(false);
	//
	// QGridLayout* layout = new QGridLayout;
	// QWidget* wrapped = new QWidget;
	// layout->addWidget(blended, 0, 0);
	// layout->addWidget(widget, 0, 0);
	// layout->setContentsMargins(0, 0, 0, 0);
	// wrapped->setLayout(layout);
	//
	// return wrapped;
	//
	return widget;
}

QWidget* _platform_macx::_osWidgetOpaque(QWidget* widget)
{
	widget->setAttribute(Qt::WA_TintedBackground);
	widget->setAutoFillBackground(true);
	return widget;
}

//TODO FIX wrong position and mouse release
void _platform_macx::_osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
{
	QWidget* tlw = widget->window();

	NSView* view = (NSView*)tlw->winId();
	NSMenu* nsMenu = menu->toNSMenu();

	QPoint globalPos = widget->mapToGlobal(pos);
	NSPoint nsPos = NSMakePoint(globalPos.x(), globalPos.y());
	nsPos = [view convertPoint:nsPos toView:view];

	menu->aboutToShow();

	/*NSEvent* event = [NSEvent
		mouseEventWithType:NSEventTypeRightMouseDown
		location:nsPos
		modifierFlags:0
		timestamp:0
		windowNumber:view ? view.window.windowNumber : 0
		context:nil
		eventNumber:0
		clickCount:1
		pressure:1.0
	];
	[NSMenu popUpContextMenu:nsMenu withEvent:event forView:view];*/

	[nsMenu popUpMenuPositioningItem:nil atLocation:nsPos inView:view];

	QMouseEvent mouseReleased(QEvent::MouseButtonRelease, tlw->pos(), tlw->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
	QCoreApplication::sendEvent(widget, &mouseReleased);

	menu->aboutToHide();
}

//TODO FIX dialogs
QLineEdit* _platform_macx::_osLineEdit(QLineEdit* input)
{
	input->setContextMenuPolicy(Qt::CustomContextMenu);
	input->connect(input, &QLineEdit::customContextMenuRequested, [=](QPoint pos) {
		QMenu* menu = input->createStandardContextMenu();
		_osContextMenuPopup(menu, input, pos);
		delete menu;
	});
	return input;
}

//TODO only for non-editable
QComboBox* _platform_macx::_osComboBox(QComboBox* select)
{
	select->setStyle(new _ComboBoxProxyStyle);
	select->setEditable(false);
	return select;
}
