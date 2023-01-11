/*!
 * e2-sat-editor/src/gui/about.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

#include "about.h"

using namespace e2se;

namespace e2se_gui_dialog
{

about::about(e2se::logger::session* log)
{
	this->log = new logger(log, "about");
	debug("about()");

	QDialog* dial = new QDialog;
	dial->setWindowTitle("About e2 SAT Editor");
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete this; });

	QGridLayout* dfrm = new QGridLayout(dial);

	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;

	QPixmap apxm = QIcon(":/icons/e2-sat-editor.png").pixmap(QSize(96, 96));

	QLabel* aicon = new QLabel;
	aicon->setPixmap(apxm);
	aicon->setAlignment(Qt::AlignTop);
	aicon->setStyleSheet("margin: 0 24px 0 10px");

	QLabel* aname = new QLabel;
	aname->setText("e2 SAT Editor");
	aname->setStyleSheet("font-size: 24px");

	QLabel* aver = new QLabel;
	aver->setText("Version 0.1");
	aver->setStyleSheet("font-size: 16px");

	QLabel* aauts = new QLabel;
	aauts->setText("e2 SAT Editor Team [Leonardo Laureti]");
	aauts->setStyleSheet("margin: 2px 0 0 0");

	QLabel* acopy = new QLabel;
	acopy->setText("Copyright 2022 © e2 SAT Editor Team [Leonardo Laureti].");
	acopy->setStyleSheet("margin: 10px 0 5px 0");

	QLabel* alise = new QLabel;
	alise->setText("This software is distributed under the terms of the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU GPLv3 License</a>.<br>\n\
The source code of this free software is available here:<br>\n\
<a href=\"https://github.com/ctlcltd/e2-sat-editor\">https://github.com/ctlcltd/e2-sat-editor</a>");
	alise->setStyleSheet("margin: 10px 0 5px 0; font-size: 10px");

	QLabel* anote = new QLabel;
	anote->setStyleSheet("font-size: 8px");
	anote->setText("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, \n\
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF \n\
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND \n\
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE \n\
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION \n\
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION \n\
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);

	dvbox->addWidget(aname);
	dvbox->addWidget(aver);
	dvbox->addWidget(aauts);
	dvbox->addWidget(acopy);
	dvbox->addWidget(alise);
	dvbox->addWidget(anote);

	dhbox->addWidget(aicon);
	dhbox->addLayout(dvbox);

	dfrm->addLayout(dhbox, 0, 0);
	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);

	dial->setLayout(dfrm);

	dial->open();
}

}
