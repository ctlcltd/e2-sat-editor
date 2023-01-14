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
#include <QStyleOption>
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

@interface _VEF_Container : NSVisualEffectView
@end

@implementation _VEF_Container

- (BOOL)isFlipped
{
	return YES;
}

@end


class _windowEventFilter : public QObject
{
	public:
		void registerEffectView(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Resize || e->type() == QEvent::Move)
			{
				QWidget* widget = qobject_cast<QWidget*>(o);
				QRect rect = widget->geometry();

				[m_view setFrameSize:[m_view.superview.window.contentView frame].size];

				std::cout << "window resizing" << ' ' << e->type() << ' ' << ' ';
				std::cout << rect.x() << ',' << rect.y() << ',' << rect.width() << ',' << rect.height() << std::endl;
			}

			return QObject::eventFilter(o, e);
		}

		NSVisualEffectView* m_view;
};


class _widgetEventFilter : public QObject
{
	public:
		void registerEffectView(NSVisualEffectView* view)
		{
			this->m_view = view;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Resize || e->type() == QEvent::Move)
			{
				if (! [m_view isHidden])
				{
					QWidget* widget = qobject_cast<QWidget*>(o);
					QPoint absolutePos = widget->pos();
					if (absolutePos.y() == 0)
					{
						absolutePos = widget->mapFromGlobal(widget->pos());
						absolutePos.setY(-absolutePos.y());
					}
					QRect rect;
					rect.setTopLeft(absolutePos);
					rect.setSize(widget->size());

					/*QWindow* top = QGuiApplication::topLevelWindows().first();
					NSView* view = (NSView*)top->winId();
					NSRect rect = [view convertRect:rect.toCGRect() fromView:view];

					[m_view setFrame:rect];*/

					NSRect nsRect = rect.toCGRect();

					[m_view setFrame:nsRect];

					std::cout << "widget resizing" << ' ' << e->type() << ' ' << ' ';
					std::cout << rect.x() << ',' << rect.y() << ',' << rect.width() << ',' << rect.height() << ' ' << ' ' << widget->window()->height() << std::endl;
				}
				else
				{
					// std::cout << "widget not resizing" << std::endl;
				}
			}
			else if (e->type() == QEvent::Show)
			{
				[m_view setHidden:FALSE];
				// std::cout << "widget shown" << std::endl;
			}
			else if (e->type() == QEvent::Hide)
			{
				[m_view setHidden:TRUE];
				// std::cout << "widget hidden" << std::endl;
			}
			else if (e->type() == QEvent::Destroy)
			{
				[m_view removeFromSuperview];
				[m_view release];
				// std::cout << "widget destroyed" << std::endl;
			}

			// int iptr = reinterpret_cast<std::uintptr_t>(this);
			// std::cout << iptr << ' ' << e->type() << std::endl;

			return QObject::eventFilter(o, e);
		}

		NSVisualEffectView* m_view;
};

QWidget* _platform_macx::_osWindowBlend(QWidget* widget, FX_MATERIAL material) {
	if (! NSClassFromString(@"NSVisualEffectView"))
		return widget;

	widget->setAttribute(Qt::WA_TranslucentBackground);

	if (_platform_macx::TESTING)
	{
		NSView* view = (NSView*)widget->winId();
		NSView* superview = view.superview;
		NSVisualEffectView* effectview = (NSVisualEffectView*)superview.subviews.firstObject;

		_VEF_Container* subview = [[_VEF_Container alloc] init];

		// [subview setTranslatesAutoresizingMaskIntoConstraints:TRUE];
		[subview setMaterial:effectview.material];
		[subview setBlendingMode:effectview.blendingMode];
		[subview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
		[subview setFrameSize:[superview.window.contentView frame].size];
		
		[effectview addSubview:subview positioned:NSWindowAbove relativeTo:nil];
		// [superview replaceSubview:effectview with:subview];

		_windowEventFilter* evt = new _windowEventFilter;
		evt->registerEffectView(subview);
		widget->installEventFilter(new _windowEventFilter);
	}
	else
	{
		NSView* view = (NSView*)widget->winId();
		NSView* superview = view.superview;
		NSVisualEffectView* subview = (NSVisualEffectView*)superview.subviews.firstObject;

		[subview setMaterial:NSVisualEffectMaterial (material)];
		[subview setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
		[subview setWantsLayer:FALSE];
	}
	
	return widget;
};

QWidget* _platform_macx::_osWidgetBlend(QWidget* widget, FX_MATERIAL material)
{
	if (_platform_macx::TESTING)
	{
		if (! NSClassFromString(@"NSVisualEffectView"))
			return widget;

		QWindow* top = QGuiApplication::topLevelWindows().first();

		widget->setAttribute(Qt::WA_TranslucentBackground);

		NSView* view = (NSView*)top->winId();
		NSView* superview = view.superview;
		NSVisualEffectView* effectview = (NSVisualEffectView*)superview.subviews.firstObject.subviews.firstObject;

		_VEF_Container* subview = [[_VEF_Container alloc] init];

		[subview setMaterial:NSVisualEffectMaterial (material)];
		[subview setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
		[subview setWantsLayer:FALSE];

		[subview setHidden:TRUE];

		[effectview addSubview:subview positioned:NSWindowAbove relativeTo:nil];

		_widgetEventFilter* evt = new _widgetEventFilter;
		evt->registerEffectView(subview);
		widget->installEventFilter(evt);
	}

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

	return widget;
}

QWidget* _platform_macx::_osWidgetOpaque(QWidget* widget)
{
	if (! _platform_macx::TESTING)
	{
		widget->setAttribute(Qt::WA_TintedBackground);
		widget->setBackgroundRole(QPalette::Window);
		widget->setAutoFillBackground(true);
	}
	return widget;
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
