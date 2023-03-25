/*!
 * e2-sat-editor/src/gui/toolkit/TreeProxyStyle.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>

#ifndef TreeProxyStyle_h
#define TreeProxyStyle_h
#include <QProxyStyle>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTreeWidget>

namespace e2se_gui
{
//TODO custom first column indentation

class TreeProxyStyle : public QProxyStyle
{
	public:
		void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// std::cout << "drawPrimitive" << ':' << ' ' << element << std::endl;

			// drawRow [QTreeView]
			// QAbstractItemView::initViewItemOption
			if (element == QStyle::PE_FrameFocusRect) // 3
			{
				return;
			}
			// drawBranch [QTreeView]
			else if (element == QStyle::PE_IndicatorBranch) // 23
			{
				const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
				QStyleOptionViewItem opt (*o);
				int indent = 10;

				if (opt.direction == Qt::LeftToRight)
					opt.rect.setX(indent);
				else if (opt.direction == Qt::RightToLeft)
					opt.rect.setX(0);
				opt.backgroundBrush = QBrush();

				return QProxyStyle::drawPrimitive(element, &opt, painter, widget);
			}
			// paintDropIndicator [QAbstractItemView]
			// handled by TreeDropIndicatorEventPainter
			else if (element == QStyle::PE_IndicatorItemViewItemDrop) // 43
			{
				return;
			}
			// drawRow [QTreeView]
			else if (element == QStyle::PE_PanelItemViewItem) // 44
			{
				const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
				QStyleOptionViewItem opt (*o);

				opt.rect.setX(0);
				// opt.backgroundBrush = QBrush(Qt::red);

				//TODO FIX
				painter->setClipping(false);

				return QProxyStyle::drawPrimitive(element, &opt, painter, widget);
			}
			// drawRow [QTreeView]
			//TODO FIX fill gap
			else if (element == QStyle::PE_PanelItemViewRow) // 45
			{
				return;
			}

			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}

		void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// std::cout << "drawControl" << ':' << ' ' << element << std::endl;

			// drawRow [QTreeView]
			if (element == QStyle::CE_ItemViewItem) // 45
			{
				const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
				QStyleOptionViewItem opt (*o);

				int indent = 10;

				if (opt.direction == Qt::LeftToRight)
					opt.rect.adjust(indent, 0, 0, 0);
				else if (opt.direction == Qt::RightToLeft)
					opt.rect.adjust(0, 0, 0, 0);

				// const QTreeWidget* tree = qobject_cast<const QTreeWidget*>(widget);

				// QModelIndex index = opt.index;
				// int indent = tree->indentation();
				// indent = index.parent().isValid() ? indent : 0;
				//
				// if (opt.direction == Qt::LeftToRight)
				// 	opt.rect.adjust(-indent, 0, 0, 0);
				//TODO FIX glitch
				// else if (opt.direction == Qt::RightToLeft)
				// 	opt.rect.adjust(0, 0, indent, 0);

				return QProxyStyle::drawControl(element, &opt, painter, widget);
			}
			// [QStyleOptionFrame]
			else if (element == QStyle::CE_ShapedFrame) // 46
			{
			}

			QProxyStyle::drawControl(element, option, painter, widget);
		}
};
}
#endif /* TreeProxyStyle_h */
