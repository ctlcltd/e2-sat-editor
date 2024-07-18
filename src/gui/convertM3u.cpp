/*!
 * e2-sat-editor/src/gui/convertM3u.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QTimer>
#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QTreeWidget>
#include <QHeaderView>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "platforms/platform.h"

#include "convertM3u.h"

using namespace e2se;

namespace e2se_gui
{

convertM3u::convertM3u(dataHandler* data)
{
	this->log = new logger("gui", "convertM3u");

	this->data = data;
}

convertM3u::~convertM3u()
{
	debug("~convertM3u");

	delete this->log;
}

void convertM3u::display(QWidget* cwid)
{
	layout(cwid);

	this->retrieve();

	dial->exec();
}

void convertM3u::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	if (this->state.dialog == dial_import)
	{
		dial->setWindowTitle(tr("Import M3U", "dialog"));

		importLayout();
	}
	else if (this->state.dialog == dial_export)
	{
		dial->setWindowTitle(tr("Export M3U", "dialog"));

		exportLayout();
	}
}

void convertM3u::toolbarLayout()
{
	debug("toolbarLayout");

	this->dtbar = toolBar();
	toolBarStyleSheet();

	this->action.cancel = toolBarAction(dtbar, tr("Cancel", "dialog"), [=]() { this->cancel(); });
	dtbar->widgetForAction(this->action.cancel)->setStyleSheet("font-size: 14px");

	toolBarSpacer(dtbar);

	if (this->state.dialog == dial_import)
		this->action.submit = toolBarAction(dtbar, tr("Import", "dialog"), theme->dynamicIcon("import"), [=]() { this->submit(); });
	else if (this->state.dialog == dial_export)
		this->action.submit = toolBarAction(dtbar, tr("Export", "dialog"), theme->dynamicIcon("export"), [=]() { this->submit(); });
}

void convertM3u::importLayout()
{
	debug("importLayout");

	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(10);
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QFormLayout* dtf10 = new QFormLayout;
	dtf10->setSpacing(10);
	dtf10->setFormAlignment(Qt::AlignLeading);
	dtf10->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth10 = new QLabel(tr("Channel group options"));
	dth10->setStyleSheet("min-height: 20px");
	dth10->setAlignment(dth10->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf10->addRow(dth10);

	QButtonGroup* dtg1 = new QButtonGroup;
	dtg1->setExclusive(true);

	QCheckBox* dtf1s0 = new QCheckBox(tr("Import each group name in Userbouquets"));
	dtf1s0->setProperty("field", "singularFalse");
	fields.emplace_back(dtf1s0);
	dtf1s0->setChecked(true);
	dtg1->addButton(dtf1s0);
	dtf10->addRow(dtf1s0);

	QCheckBox* dtf1s1 = new QCheckBox(tr("Import all entries in one Userbouquet"));
	dtf1s1->setProperty("field", "singularTrue");
	fields.emplace_back(dtf1s1);
	dtg1->addButton(dtf1s1);
	dtf10->addRow(dtf1s1);

	dtf0->addRow(dtf10);
	dtf0->addItem(new QSpacerItem(0, 0));

	QFormLayout* dtf20 = new QFormLayout;
	dtf20->setSpacing(15);
	dtf20->setFormAlignment(Qt::AlignLeading);
	dtf20->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf2dr = new QCheckBox(tr("Discard Reference ID if set"));
	dtf2dr->setProperty("field", "discardId");
	fields.emplace_back(dtf2dr);
	dtf20->addRow(dtf2dr);

	dtf0->addRow(dtf20);

	//TODO download channel logos
	// this is a hot thing, these m3u are known to contain unsafe data.
	// a solution could be after import dialog, filter for trusted hosts,
	// tree view, multiple selection, opening and viewing URLs on the browser

	/*dtf0->addItem(new QSpacerItem(0, 0));

	QFormLayout* dtf30 = new QFormLayout;
	dtf30->setSpacing(12);
	dtf30->setFormAlignment(Qt::AlignLeading);
	dtf30->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf3ld = new QCheckBox(tr("Download channel logos"));
	dtf3ld->setProperty("field", "downloadChLogos");
	fields.emplace_back(dtf3ld);
	dtf30->addRow(dtf3ld);

	QWidget* dtf3lw = new QWidget;

	QFormLayout* dtf3lf = new QFormLayout;
	dtf3lf->setFormAlignment(Qt::AlignLeading);
	dtf3lf->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* dtf3lh = new QHBoxLayout;
	dtf3lh->setAlignment(dtf3lw->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);

	QLineEdit* dtf3lp = new QLineEdit;
	dtf3lp->setProperty("field", "chLogosBrowsePath");
	fields.emplace_back(dtf3lp);
	dtf3lp->setReadOnly(true);
	platform::osLineEdit(dtf3lp);

	QPushButton* dtf3lb = new QPushButton;
	dtf3lb->setText(tr("&Browse…", "toolbar"));
	dtf3lb->connect(dtf3lb, &QPushButton::pressed, [=]() {
		QString curr_dir = QSettings().value("application/m3uChLogosBrowsePath").toString();
		QString dir = this->browseFileDialog(curr_dir);
		dtf3lp->setText(dir);
	});

	dtf3lh->addWidget(dtf3lp);
	dtf3lh->addWidget(dtf3lb);
	dtf3lf->addRow(tr("Folder", "toolbar"), dtf3lh);

	dtf3lw->setLayout(dtf3lf);
	dtf30->addWidget(dtf3lw);

	dtf0->addRow(dtf30);*/

	dtform->addLayout(dtf0, 0, 0);
}

void convertM3u::exportLayout()
{
	debug("exportLayout");

	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(10);
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	if (this->ubouquets.size() > 1)
	{
		auto* dbih = this->data->dbih;

		QGroupBox* dtl1 = new QGroupBox(tr("Select Userbouquets"));
		dtl1->setStyleSheet("QGroupBox { spacing: 0; padding: 0; padding-top: 25px; border: 0; } QGroupBox::title { margin: 0 }");
		QVBoxLayout* dtb1 = new QVBoxLayout;
		dtb1->setContentsMargins(0, 0, 0, 0);

		this->ubt = new QTreeWidget;
		ubt->setIndentation(0);
		ubt->connect(ubt, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
			if (previous != nullptr && current != nullptr)
				current->setCheckState(0, current->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
		});

		ubt->setHeaderLabels({NULL, tr("Name"), tr("Filename")});
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
		ubt->header()->setDefaultSectionSize(0);
#endif
		ubt->setColumnWidth(0, 33);
		ubt->setColumnWidth(1, 200);
		ubt->setColumnWidth(2, 150);
#ifdef Q_OS_WIN
		if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
		{
			QStyle* style;

			if (theme::isFluentWin())
				style = QStyleFactory::create("windows11");
			else
				style = QStyleFactory::create("fusion");

			ubt->verticalScrollBar()->setStyle(style);
			ubt->horizontalScrollBar()->setStyle(style);
			ubt->header()->setStyle(style);
		}
#endif

		QList<QTreeWidgetItem*> btree;
		for (string & iname : this->ubouquets)
		{
			e2db::userbouquet uboq = dbih->userbouquets[iname];
			QString name = QString::fromStdString(uboq.name);
			QString filename = QString::fromStdString(uboq.rname.empty() ? uboq.bname : uboq.rname);
			QString bname = QString::fromStdString(uboq.bname);

			QTreeWidgetItem* item = new QTreeWidgetItem;
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
			item->setText(1, name);
			item->setText(2, filename);
			item->setData(0, Qt::UserRole, bname);

			if (this->opts.fc == e2db::FCONVS::convert_current && this->opts.bname == uboq.bname)
				item->setCheckState(0, Qt::Checked);
			else
				item->setCheckState(0, Qt::Unchecked);

			btree.append(item);
		}
		ubt->addTopLevelItems(btree);

		dtb1->addWidget(ubt);

		dtl1->setLayout(dtb1);
		dtf0->addRow(dtl1);
	}

	QFormLayout* dtf10 = new QFormLayout;
	dtf10->setSpacing(10);
	dtf10->setFormAlignment(Qt::AlignLeading);
	dtf10->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth10 = new QLabel(tr("Channel group options"));
	dth10->setStyleSheet("min-height: 20px");
	dth10->setAlignment(dth10->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf10->addRow(dth10);

	QButtonGroup* dtg1 = new QButtonGroup;
	dtg1->setExclusive(true);

	QCheckBox* dtf1s0 = new QCheckBox(tr("Import each group name in Userbouquets"));
	dtf1s0->setProperty("field", "singularFalse");
	fields.emplace_back(dtf1s0);
	dtf1s0->setChecked(true);
	dtg1->addButton(dtf1s0);
	dtf10->addRow(dtf1s0);

	QCheckBox* dtf1s1 = new QCheckBox(tr("Import all entries in one Userbouquet"));
	dtf1s1->setProperty("field", "singularTrue");
	fields.emplace_back(dtf1s1);
	dtg1->addButton(dtf1s1);
	dtf10->addRow(dtf1s1);

	dtf0->addRow(dtf10);
	dtf0->addItem(new QSpacerItem(0, 0));

	QFormLayout* dtf20 = new QFormLayout;
	dtf20->setSpacing(15);
	dtf20->setFormAlignment(Qt::AlignLeading);
	dtf20->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf2cr = new QCheckBox(tr("Set channel Reference ID"));
	dtf2cr->setProperty("field", "chReferenceId");
	fields.emplace_back(dtf2cr);
	dtf2cr->setChecked(true);
	dtf20->addRow(dtf2cr);

	QCheckBox* dtf2cn = new QCheckBox(tr("Set channel number"));
	dtf2cn->setProperty("field", "chNumber");
	fields.emplace_back(dtf2cn);
	dtf2cn->setChecked(true);
	dtf20->addRow(dtf2cn);

	QCheckBox* dtf2cg = new QCheckBox(tr("Set channel group name"));
	dtf2cg->setProperty("field", "chGroupName");
	fields.emplace_back(dtf2cg);
	dtf2cg->setChecked(true);
	dtf20->addRow(dtf2cg);

	dtf0->addRow(dtf20);

	//TODO export channel logos
	// it is hardly feasible to upload logos to any hosts,
	// but you can prepare URLs to be uploaded later.
	// a solution could be an additional before export dialog,
	// tree view, pre-filled host and editable URLs

	dtf0->addItem(new QSpacerItem(0, 0));

	QFormLayout* dtf30 = new QFormLayout;
	dtf30->setSpacing(15);
	dtf30->setFormAlignment(Qt::AlignLeading);
	dtf30->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf3cl = new QCheckBox(tr("Set channel logos URL placeholder"));
	dtf3cl->setProperty("field", "chLogosPlaceholder");
	fields.emplace_back(dtf3cl);
	dtf30->addRow(dtf3cl);

	QLineEdit* dtf3lu = new QLineEdit;
	dtf3lu->setProperty("field", "chLogosBaseURL");
	fields.emplace_back(dtf3lu);
	dtf3lu->setMinimumWidth(200);
	platform::osLineEdit(dtf3lu);
	dtf30->addRow(tr("Channel logos base URL"), dtf3lu);

	dtf0->addRow(dtf30);

	dtform->addLayout(dtf0, 0, 0);
}

void convertM3u::store()
{
	debug("store");

	int flags = 0;

	if (this->state.dialog == dial_import)
		flags = e2db::M3U_FLAGS::m3u_default;
	else if (this->state.dialog == dial_export)
		flags = 0;

	for (auto & item : fields)
	{
		QString key = item->property("field").toString();
		QVariant val;

		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			val = field->isChecked();
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text();

		if (this->state.dialog == dial_import)
		{
			if (key == "singularTrue" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_singular;
			else if (key == "discardId" && val.toBool())
				flags &= e2db::M3U_FLAGS::m3u_chrefid;
			/*else if (key == "downloadChLogos" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_chlogos;
			else if (key == "chLogosBrowsePath")
				QSettings().setValue("application/m3uChLogosBrowsePath", val.toString());*/
		}
		else if (this->state.dialog == dial_export)
		{
			if (key == "singularTrue" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_singular;
			else if (key == "chReferenceId" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_chrefid;
			else if (key == "chNumber" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_chnum;
			else if (key == "chGroupName" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_chgroup;
			else if (key == "chLogosPlaceholder" && val.toBool())
				flags |= e2db::M3U_FLAGS::m3u_chlogos;
			else if (key == "chLogosBaseURL")
				QSettings().setValue("application/m3uChLogosBaseURL", val.toString());
		}
	}

	if (this->state.dialog == dial_import)
		QSettings().setValue("application/m3uImportFlags", flags);
	else if (this->state.dialog == dial_export)
		QSettings().setValue("application/m3uExportFlags", flags);

	this->opts.flags = flags;

	if (this->state.dialog == dial_export)
	{
		if (this->opts.flags & e2db::M3U_FLAGS::m3u_chlogos)
			this->opts.logosbase = QSettings().value("application/m3uChLogosBaseURL").toString().toStdString();

		vector<string> ubouquets;

		int i = 0;
		int j = ubt->topLevelItemCount();

		while (i != j)
		{
			QTreeWidgetItem* item = ubt->topLevelItem(i);

			if (item->checkState(0) == Qt::Checked)
				ubouquets.emplace_back(item->data(0, Qt::UserRole).toString().toStdString());

			i++;
		}

		this->ubouquets.swap(ubouquets);
	}

	this->changes = true;
}

void convertM3u::retrieve()
{
	debug("retrieve");

	int flags = 0;

	if (this->state.dialog == dial_import)
		flags = QSettings().value("application/m3uImportFlags", e2db::M3U_FLAGS::m3u_default).toInt();
	else if (this->state.dialog == dial_export)
		flags = QSettings().value("application/m3uExportFlags", e2db::M3U_FLAGS::m3u_default).toInt();

	for (auto & item : fields)
	{
		QString key = item->property("field").toString();
		QVariant val;

		if (this->state.dialog == dial_import)
		{
			if (key == "singularTrue")
				val = (flags & e2db::M3U_FLAGS::m3u_singular);
			else if (key == "singularFalse")
				val = ! (flags & e2db::M3U_FLAGS::m3u_singular);
			else if (key == "discardId")
				val = ! (flags & e2db::M3U_FLAGS::m3u_chrefid);
			/*else if (key == "downloadChLogos")
				val = (flags & e2db::M3U_FLAGS::m3u_chlogos);
			else if (key == "chLogosBrowsePath")
				val = QSettings().value("application/m3uChLogosBrowsePath");*/
		}
		else if (this->state.dialog == dial_export)
		{
			if (key == "singularTrue")
				val = (flags & e2db::M3U_FLAGS::m3u_singular);
			else if (key == "singularFalse")
				val = ! (flags & e2db::M3U_FLAGS::m3u_singular);
			else if (key == "chReferenceId")
				val = (flags & e2db::M3U_FLAGS::m3u_chrefid);
			else if (key == "chNumber")
				val = (flags & e2db::M3U_FLAGS::m3u_chnum);
			else if (key == "chGroupName")
				val = (flags & e2db::M3U_FLAGS::m3u_chgroup);
			else if (key == "chLogosPlaceholder")
				val = (flags & e2db::M3U_FLAGS::m3u_chlogos);
			else if (key == "chLogosBaseURL")
				val = QSettings().value("application/m3uChLogosBaseURL");
		}

		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			field->setChecked(val.toBool());
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(val.toString());
	}
}

void convertM3u::setImport(e2db::fcopts& opts)
{
	debug("setImport");

	this->state.dialog = DIAL::dial_import;
	this->opts = opts;
}

void convertM3u::setExport(e2db::fcopts& opts, vector<string>& ubouquets)
{
	debug("setExport");

	this->state.dialog = DIAL::dial_export;
	this->opts = opts;
	this->ubouquets = ubouquets;
}

e2db::fcopts convertM3u::getConverterOptions()
{
	debug("getConverterOptions");

	return this->opts;
}

QString convertM3u::browseFileDialog(QString path)
{
#ifndef E2SE_DEMO
	return QFileDialog::getExistingDirectory(nullptr, tr("Select channel logos folder", "file-dialog"), path);
#else
	return "";
#endif
}

}
