/*!
 * e2-sat-editor/src/gui/toolkit/TreeProxyStyle.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef TreeProxyStyle_h
#define TreeProxyStyle_h
#include <QtGlobal>
#include <QProxyStyle>
#include <QPainter>

#include "../theme.h"

namespace e2se_gui
{
class TreeProxyStyle : public QProxyStyle
{
	public:
		void setIndentation(int indent, bool firstColumnIndented = false)
		{
			this->indent = indent;
			this->firstColumnIndented = firstColumnIndented;
		}
		int indentation()
		{
			return this->indent;
		}
		void setFirstColumnIndent(int column)
		{
			this->firstColumn = column;
		}
		int firstColumnIndent()
		{
			return this->firstColumn;
		}
		void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// QAbstractItemView::initViewItemOption
			if (element == QStyle::PE_FrameFocusRect) // 3
				return;
			// drawBranch [QTreeView]
			// drawn by drawPrimitivePanelItemViewItem
			else if (element == QStyle::PE_IndicatorBranch) // 23
				return;
			// paintDropIndicator [QAbstractItemView]
			// drawn by e2se_gui::TreeDropIndicatorEventPainter
			else if (element == QStyle::PE_IndicatorItemViewItemDrop) // 43
				return;
			// drawRow [QTreeView]
			else if (element == QStyle::PE_PanelItemViewItem) // 44
				return drawPrimitivePanelItemViewItem(option, painter, widget);
			// drawRow [QTreeView]
			else if (element == QStyle::PE_PanelItemViewRow) // 45
				return;

			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
		void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// drawRow [QTreeView]
			if (element == QStyle::CE_ItemViewItem) // 45
				return drawControlItemViewItem(option, painter, widget);

			QProxyStyle::drawControl(element, option, painter, widget);
		}

	protected:
		int indent = 0;
		int firstColumn = 0;
		bool firstColumnIndented = false;

		void drawPrimitiveIndicatorBranch(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			opt.rect = QRect(0, opt.rect.y(), opt.rect.height(), opt.rect.height());
			opt.backgroundBrush = QBrush();

			int dx = 0;

#ifdef Q_OS_WIN
			if (! theme::isOverridden() && theme::isFluentWin())
			{
				dx = -1;
			}
#endif

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				dx += gap;
			}

			if (dx)
			{
				if (opt.direction == Qt::LeftToRight)
				{
					opt.rect.setX(- (opt.rect.height() / 2));
					opt.rect.adjust(dx - 2, 0, 0, 0);
				}
				else if (opt.direction == Qt::RightToLeft)
				{
					opt.rect.setX(option->rect.right() * 2 - opt.rect.height());
					opt.rect.adjust(0, 0, dx + 2 + 2, 0);
				}
			}

			QProxyStyle::drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, widget);
		}
		void drawPrimitivePanelItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			int dx = this->firstColumnIndented ? (index.column() == this->firstColumn ? this->indent : 0) : this->indent;

#ifdef Q_OS_WIN
			if (! theme::isOverridden() && theme::isFluentWin())
			{
				dx = this->indent;
			}
#endif

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				dx += gap;
			}

			if (dx)
			{
				if (opt.direction == Qt::LeftToRight)
				{
					opt.rect.adjust(-dx, 0, 0, 0);
				}
				else if (opt.direction == Qt::RightToLeft)
				{
					if (this->firstColumnIndented)
						opt.rect.setWidth(option->rect.right() * 2);

					opt.rect.adjust(0, 0, dx, 0);
				}

				painter->setClipRect(opt.rect);
			}

			QProxyStyle::drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

			if (opt.state & QProxyStyle::State_Children)
				drawPrimitiveIndicatorBranch(option, painter, widget);
		}
		void drawControlItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			int dx = this->firstColumnIndented ? (index.column() == this->firstColumn ? this->indent : 0) : this->indent;

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				dx += gap;
			}

			if (dx)
			{
				if (opt.direction == Qt::LeftToRight)
				{
					opt.rect.adjust(dx, 0, 0, 0);
				}
				else if (opt.direction == Qt::RightToLeft)
				{
					opt.rect.adjust(0, 0, -dx, 0);
				}
			}

			QProxyStyle::drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
		}
};
}
#endif /* TreeProxyStyle_h */
