/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#import <AppKit/AppKit.h>

#include <iostream>

#include <QtGui>
#include <QProxyStyle>
#include <QStyleOption>
#include <QTimer>
#include <QGridLayout>

#include "platform_macx.h"


@interface BackgroundView : NSVisualEffectView
@end

@implementation BackgroundView

- (BOOL)isFlipped
{
	return YES;
}

@end


class _windowEventFilter : public QObject
{
	public:
		_windowEventFilter(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Move)
			{
				NSView* superview = m_view.superview;

				[m_view setFrame:superview.window.contentView.frame]; // fullscreen needed
			}
			else if (e->type() == QEvent::Destroy)
			{
				[m_view removeFromSuperview];
				[m_view release];
			}

			// std::cout << "window QEvent" << ' ' << e->type() << std::endl;
			return QObject::eventFilter(o, e);
		}

		NSVisualEffectView* m_view;
};


class _widgetEventFilter : public QObject
{
	public:
		_widgetEventFilter(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Resize || e->type() == QEvent::Move)
			{
				QWidget* widget = qobject_cast<QWidget*>(o);
				QPoint framePos = widget->mapTo(widget->window(), QPoint());
				QRect frameRect;
				frameRect.setTopLeft(framePos);
				frameRect.setSize(widget->size());

				NSRect nsFrame = frameRect.toCGRect();

				[m_view setFrame:nsFrame];

				// std::cout << "widget resizing" << ' ' << e->type() << ' ' << ' ' << frameRect.top() << ',' << frameRect.left() << ',' << frameRect.width() << ',' << frameRect.height() << ' ' << ' ' << widget->window()->width() << ',' << widget->window()->height() << std::endl;
			}
			else if (e->type() == QEvent::Show)
			{
				QWidget* widget = qobject_cast<QWidget*>(o);
				QWindow* top = widget->window()->windowHandle();

				if (top != nullptr)
				{
					NSView* view = (NSView*)top->winId();
					NSView* superview = view.superview;
					NSVisualEffectView* backgroundview = (NSVisualEffectView*)superview.subviews.firstObject.subviews.firstObject;

					[backgroundview addSubview:m_view positioned:NSWindowAbove relativeTo:nil];
				}

				[m_view setHidden:FALSE];
			}
			else if (e->type() == QEvent::Hide)
			{
				[m_view setHidden:TRUE];
			}
			else if (e->type() == QEvent::Destroy)
			{
				[m_view removeFromSuperview];
				[m_view release];
			}

			// std::cout << "widget QEvent" << ' ' << e->type() << std::endl;
			return QObject::eventFilter(o, e);
		}

		NSVisualEffectView* m_view;
};


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


QWidget* _platform_macx::_osWindowBlend(QWidget* widget) {
	if (! NSClassFromString(@"NSVisualEffectView"))
		return widget;

	// Qt::WA_TranslucentBackground should be set before QWidget::winId()
	// to avoid painting glitch
	widget->setAttribute(Qt::WA_TranslucentBackground);

	NSView* view;

	if (widget->windowType() == Qt::Window)
	{
		view = (NSView*)widget->winId();
	}
	//TODO FIX
	else if (widget->windowType() == Qt::Dialog)
	{
		// view = (NSView*)widget;
		return widget;
	}
	else
	{
		return widget;
	}

	NSView* superview = view.superview;
	NSVisualEffectView* effectview = (NSVisualEffectView*)superview.subviews.firstObject;

	BackgroundView* subview = [[BackgroundView alloc] init];

	[subview setMaterial:effectview.material];
	[subview setBlendingMode:effectview.blendingMode];
	[subview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	[subview setFrame:superview.window.contentView.frame]; // will autoresize
	
	[effectview addSubview:subview positioned:NSWindowAbove relativeTo:nil];

	widget->installEventFilter(new _windowEventFilter(subview));

	return widget;
};

QWidget* _platform_macx::_osWidgetBlend(QWidget* widget, FX_MATERIAL material, FX_BLENDING blending)
{
	if (! NSClassFromString(@"NSVisualEffectView"))
		return widget;

	widget->setAttribute(Qt::WA_TranslucentBackground);

	BackgroundView* subview = [[BackgroundView alloc] init];

	[subview setMaterial:NSVisualEffectMaterial (material)];
	[subview setBlendingMode:NSVisualEffectBlendingMode (blending)];
	[subview setWantsLayer:FALSE];
	[subview setHidden:TRUE];

	widget->installEventFilter(new _widgetEventFilter(subview));

	return widget;

	
	//WONTFIX performance issues
	// use _platform_macx::osWindowBlend instead
	//
	// huge impact (~400MB RAM with main window maximized)
	// native widgets and backing store
	// see QWidget documentation for more details
	// section: Native Widgets vs Alien Widgets
	//
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
}

//TODO FIX wrong position and mouse release
void _platform_macx::_osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
{
	QWidget* top = widget->window();

	NSView* view = (NSView*)top->winId();
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

	QMouseEvent mouseReleased(QEvent::MouseButtonRelease, top->pos(), top->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
	QGuiApplication::sendEvent(widget, &mouseReleased);

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
