/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
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


class _windowEventObserver : public QObject
{
	public:
		_windowEventObserver(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event)
		{
			if (event->type() == QEvent::Move)
			{
				NSView* superview = m_view.superview;

				[m_view setFrame:superview.window.contentView.frame]; // fullscreen needed
			}
			else if (event->type() == QEvent::Destroy)
			{
				[m_view removeFromSuperview];
				[m_view release];
			}

			// std::cout << "window QEvent" << ' ' << event->type() << std::endl;
			return QObject::eventFilter(object, event);
		}

		NSVisualEffectView* m_view;
};


class _widgetEventObserver : public QObject
{
	public:
		_widgetEventObserver(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event)
		{
			if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
			{
				QWidget* widget = qobject_cast<QWidget*>(object);
				QPoint framePos = widget->mapTo(widget->window(), QPoint());
				QRect frameRect;
				frameRect.setTopLeft(framePos);
				frameRect.setSize(widget->size());

				NSRect nsFrame = frameRect.toCGRect();

				[m_view setFrame:nsFrame];

				// std::cout << "widget resizing" << ' ' << event->type() << ' ' << ' ' << frameRect.top() << ',' << frameRect.left() << ',' << frameRect.width() << ',' << frameRect.height() << ' ' << ' ' << widget->window()->width() << ',' << widget->window()->height() << std::endl;
			}
			else if (event->type() == QEvent::Show)
			{
				QWidget* widget = qobject_cast<QWidget*>(object);
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
			else if (event->type() == QEvent::Hide)
			{
				[m_view setHidden:TRUE];
			}
			else if (event->type() == QEvent::Destroy)
			{
				[m_view removeFromSuperview];
				[m_view release];
			}

			// std::cout << "widget QEvent" << ' ' << event->type() << std::endl;
			return QObject::eventFilter(object, event);
		}

		NSVisualEffectView* m_view;
};

class _osPersistentEditorEventObserver : public QObject
{
	protected:
		bool eventFilter(QObject* object, QEvent* event)
		{
			QWidget* widget = qobject_cast<QWidget*>(object);

			if (! widget)
				return QObject::eventFilter(object, event);

			if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && widget->isWindow())) {
				if (! widget->isActiveWindow() || (QApplication::focusWidget() != widget)) {
					QWidget* w = QApplication::focusWidget();

					if (w == widget)
						return QObject::eventFilter(object, event);

					if (QLineEdit* input = qobject_cast<QLineEdit*>(w))
						_platform_macx::osLineEdit(input, false);
				}
			}

			return QObject::eventFilter(object, event);
		}
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
	else if (widget->windowType() == Qt::Dialog)
	{
		//workaround: QNSPanel should be initialized winId is 0x0
		widget->show();
		widget->hide();

		view = (NSView*)widget->winId();
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

	widget->installEventFilter(new _windowEventObserver(subview));

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

	widget->installEventFilter(new _widgetEventObserver(subview));

	return widget;
}

void _platform_macx::_osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
{
	QWidget* top = widget->window();
	QWindow* tlw = top->windowHandle();

	NSView* view = (NSView*)top->winId();
	NSMenu* nsMenu = menu->toNSMenu();

	QPoint globalPos = widget->mapTo(top, pos);

	NSPoint nsPos = NSMakePoint(globalPos.x(), globalPos.y());
	nsPos = [view convertPoint:nsPos toView:nil];

	// signal emit
	menu->aboutToShow();

	NSEvent* nsEventMenuShow = [NSEvent
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

	//workaround: inner items disabled with modals
	if (tlw != nullptr && tlw->type() != Qt::Window)
	{
		[nsMenu setAutoenablesItems:FALSE];

		int i = 0;
		for (auto & item : menu->actions())
		{
			NSMenuItem* nsMenuItem = [nsMenu itemAtIndex:i++];
			[nsMenuItem setEnabled:(item->isEnabled())];
		}
	}

	[NSMenu popUpContextMenu:nsMenu withEvent:nsEventMenuShow forView:view];

	NSEvent* nsEventMouseRelease = [NSEvent
		mouseEventWithType:NSEventTypeRightMouseUp
		location:nsPos
		modifierFlags:0
		timestamp:0
		windowNumber:view ? view.window.windowNumber : 0
		context:nil
		eventNumber:0
		clickCount:1
		pressure:1.0
	];
	[view mouseUp:nsEventMouseRelease];

	// signal emit
	menu->aboutToHide();
}

//TODO improve native macx context menu items
QLineEdit* _platform_macx::_osLineEdit(QLineEdit* input, bool destroy)
{
	input->setContextMenuPolicy(Qt::CustomContextMenu);
	input->connect(input, &QLineEdit::customContextMenuRequested, [=](QPoint pos) {
		QMenu* menu = input->createStandardContextMenu();
		_osContextMenuPopup(menu, input, pos);

		if (destroy)
		{
			// menu delete after QAction signal trigger
			QTimer::singleShot(300, [=]() {
				menu->deleteLater();
			});
		}
	});
	return input;
}

QComboBox* _platform_macx::_osComboBox(QComboBox* select)
{
	select->setStyle(new _ComboBoxProxyStyle);
	select->setEditable(false);
	return select;
}

//TODO improve native macx context menu items
QTextEdit* _platform_macx::_osTextEdit(QTextEdit* input, bool destroy)
{
	input->setContextMenuPolicy(Qt::CustomContextMenu);
	input->connect(input, &QLineEdit::customContextMenuRequested, [=](QPoint pos) {
		QMenu* menu = input->createStandardContextMenu();
		_osContextMenuPopup(menu, input, pos);

		if (destroy)
		{
			// menu delete after QAction signal trigger
			QTimer::singleShot(300, [=]() {
				menu->deleteLater();
			});
		}
	});
	return input;
}

//TODO improve native macx context menu items
QWidget* _platform_macx::_osPersistentEditor(QWidget* widget)
{
	widget->installEventFilter(new _osPersistentEditorEventObserver);
	return widget;
}