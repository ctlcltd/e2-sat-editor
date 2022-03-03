/*!
 * e2-sat-editor/src/gui/about.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "../commons.h"
#include "about.h"

using namespace std;

namespace e2se_gui_dialog
{
void about(QWidget* mwid)
{
	debug("gui", "about()");

	QDialog* dial = new QDialog(mwid);
	dial->setWindowTitle("About e2-sat-editor");

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;

	QLabel* aicon = new QLabel;
	aicon->setText("📡");
	aicon->setStyleSheet("margin: 0 16px 96px 12px; font-size: 64px");
	QLabel* aname = new QLabel;
	aname->setText("e2-sat-editor");
	aname->setStyleSheet("font-size: 24px");
	QLabel* aver = new QLabel;
	aver->setText("Version 0.1");
	aver->setStyleSheet("font-size: 16px");
	QLabel* aauts = new QLabel;
	aauts->setText("e2-sat-editor Team (Leonardo Laureti)");
	aauts->setStyleSheet("margin: 2px 0 0 0");
	QLabel* acopy = new QLabel;
	acopy->setText("Copyright © e2-sat-editor Team (Leonardo Laureti).");
	acopy->setStyleSheet("margin: 10px 0 5px 0");
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
	dvbox->addWidget(anote);

	dhbox->addWidget(aicon);
	dhbox->addLayout(dvbox);

	dfrm->addLayout(dhbox, 0, 0);
	dial->setLayout(dfrm);
	dial->exec();
}
}
