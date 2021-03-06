/*!
 * e2-sat-editor/src/gui/toolkit/TreeStyledItemDelegate.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef TreeStyledItemDelegate_h
#define TreeStyledItemDelegate_h
#include <QStyledItemDelegate>

namespace e2se_gui
{
class TreeStyledItemDelegate : public QStyledItemDelegate
{
	using QStyledItemDelegate::QStyledItemDelegate;

	public:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			QStyleOptionViewItem opt (option);
			if (index.parent().isValid() && (! index.model() || ! index.model()->index(0, 0, index).isValid()))
				opt.rect.adjust(-this->indent, 0, 0, 0);

			QStyledItemDelegate::QStyledItemDelegate::paint(painter, opt, index);
		}
		void setIndentation(int i)
		{
			this->indent = i;
		}
		int indentation()
		{
			return this->indent;
		}
	protected:
		int indent;
};
}
#endif /* TreeStyledItemDelegate_h */
