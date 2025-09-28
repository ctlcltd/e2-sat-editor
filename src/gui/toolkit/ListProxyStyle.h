/*!
 * e2-sat-editor/src/gui/toolkit/ListProxyStyle.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ListProxyStyle_h
#define ListProxyStyle_h
#include <QProxyStyle>
#include <QPainter>

namespace e2se_gui
{
class ListProxyStyle : public QProxyStyle
{
	public:
		void setIndentation(int indent)
		{
			this->indent = indent;
		}
		int indentation()
		{
			return this->indent;
		}
		void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// QAbstractItemView::initViewItemOption
			if (element == QStyle::PE_FrameFocusRect) // 3
				return;
			// QStyledItemDelegate::paint
			else if (element == QStyle::PE_PanelItemViewItem) // 44
				return drawPrimitivePanelItemViewItem(option, painter, widget);

			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
		void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			// paintEvent [QListView]
			if (element == QStyle::CE_ItemViewItem) // 45
				return drawControlItemViewItem(option, painter, widget);

			QProxyStyle::drawControl(element, option, painter, widget);
		}

	protected:
		int indent = 0;

		void drawPrimitivePanelItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);

			if (opt.direction == Qt::LeftToRight)
				opt.rect.adjust(-indent, 0, 0, 0);
			else if (opt.direction == Qt::RightToLeft)
				opt.rect.adjust(0, 0, indent, 0);

			painter->setClipRect(opt.rect);

			QProxyStyle::drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
		}
		void drawControlItemViewItem(const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const
		{
			const QStyleOptionViewItem* o = qstyleoption_cast<const QStyleOptionViewItem*>(option);
			QStyleOptionViewItem opt (*o);

			if (opt.direction == Qt::LeftToRight)
				opt.rect.adjust(indent, 0, 0, 0);
			else if (opt.direction == Qt::RightToLeft)
				opt.rect.adjust(0, 0, -indent, 0);

			QProxyStyle::drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
		}
};
}
#endif /* ListProxyStyle_h */
