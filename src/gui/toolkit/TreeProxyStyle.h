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
#include <QPainter>

namespace e2se_gui
{
//TODO
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
			// std::cout << "drawPrimitive" << ':' << ' ' << element << std::endl;

			// QAbstractItemView::initViewItemOption
			if (element == QStyle::PE_FrameFocusRect) // 3
				return;
			// drawBranch [QTreeView]
			else if (element == QStyle::PE_IndicatorBranch) // 23
				return drawPrimitiveIndicatorBranch(option, painter, widget);
			// paintDropIndicator [QAbstractItemView]
			// handled by TreeDropIndicatorEventPainter
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
			// std::cout << "drawControl" << ':' << ' ' << element << std::endl;

			// drawRow [QTreeView]
			if (element == QStyle::CE_ItemViewItem) // 45
				return drawControlItemViewItem(option, painter, widget);

			QProxyStyle::drawControl(element, option, painter, widget);
		}

	protected:
		int indent = 0;
		int firstColumn = 0;
		bool firstColumnIndented = false;

		//TODO branch indicator width gap
		void drawPrimitiveIndicatorBranch(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			int indent = this->firstColumnIndented ? (index.column() == this->firstColumn ? this->indent : 0) : this->indent;

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				indent -= gap;
			}

			if (indent)
			{
				if (opt.direction == Qt::LeftToRight)
					opt.rect.adjust(indent, 0, 0, 0);
				else if (opt.direction == Qt::RightToLeft)
					opt.rect.adjust(0, 0, -indent, 0);
			}

			opt.backgroundBrush = QBrush();

			QProxyStyle::drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, widget);
		}
		void drawPrimitivePanelItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			int indent = this->firstColumnIndented ? (index.column() == this->firstColumn ? this->indent : 0) : this->indent;

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				indent += gap;
			}

			if (indent)
			{
				if (opt.direction == Qt::LeftToRight)
					opt.rect.adjust(-indent, 0, 0, 0);
				else if (opt.direction == Qt::RightToLeft)
				{
					if (this->firstColumnIndented)
						opt.rect.adjust(0, 0, indent, 0);
					//TODO FIX tree->indentation() * depth - 1px rtl glitch
					else
					{
						opt.rect.setX(-1);
						opt.rect.setWidth(opt.rect.width() + this->indent);
					}
				}

				painter->setClipRect(opt.rect);
			}

			QProxyStyle::drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
		}
		void drawControlItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);
			QModelIndex index = opt.index;

			int indent = this->firstColumnIndented ? (index.column() == this->firstColumn ? this->indent : 0) : this->indent;

			if (! this->firstColumnIndented)
			{
				int gap = 1;
				for (auto i = index; (i = i.parent()).isValid(); gap++);
				indent -= gap;
			}

			if (indent)
			{
				if (opt.direction == Qt::LeftToRight)
					opt.rect.adjust(indent, 0, 0, 0);
				else if (opt.direction == Qt::RightToLeft)
				{
					if (this->firstColumnIndented)
						opt.rect.adjust(0, 0, -indent, 0);
					//TODO FIX tree->indentation() * depth - 1px rtl glitch
					else
						opt.rect.adjust(0, 0, -this->indent, 0);
				}
			}

			QProxyStyle::drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
		}
};
}
#endif /* TreeProxyStyle_h */
