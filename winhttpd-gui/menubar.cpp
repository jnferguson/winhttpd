#include "menubar.h"

menubar_t::menubar_t(QMainWindow* p) : QMenuBar(p)
{
	m_menuFile			= new QMenu(this);
	m_menuFileService	= m_menuFile->addMenu("Ser&vice");
	m_menuEdit			= new QMenu(this);
	m_menuHelp			= new QMenu(this);
	m_menuSettings		= new QAction(p);
	m_menuExit			= new QAction(p);
	m_editCut			= new QAction(p);
	m_editCopy			= new QAction(p);
	m_editSelectAll		= new QAction(p);
	m_editPaste			= new QAction(p);
	m_helpHelp			= new QAction(p);
	m_helpAbout			= new QAction(p);
	m_startService		= new QAction(p);
	m_stopService		= new QAction(p);
	m_pauseService		= new QAction(p);
	m_resumeService		= new QAction(p);

	setGeometry(QRect(0, 0, 800, 21));
	addAction(m_menuFile->menuAction());
	addAction(m_menuEdit->menuAction());
	addAction(m_menuHelp->menuAction());

	m_menuFile->setTitle("&File");
	m_menuEdit->setTitle("&Edit");
	m_menuHelp->setTitle("&Help");

	m_editCut->setText("Cut");
	m_editCut->setShortcut(QKeySequence::Cut);
	m_editCut->setStatusTip("Cut");
	m_menuEdit->addAction(m_editCut);
	connect(m_editCut, &QAction::triggered, this, &menubar_t::cutTriggered);
	
	m_editCopy->setText("Copy");
	m_editCopy->setShortcut(QKeySequence::Copy);
	m_editCopy->setStatusTip("Copy");
	m_menuEdit->addAction(m_editCopy);
	connect(m_editCopy, &QAction::triggered, this, &menubar_t::copyTriggered);

	m_editSelectAll->setText("Select All");
	m_editSelectAll->setShortcut(QKeySequence::SelectAll);
	m_editSelectAll->setStatusTip("Select All");
	m_menuEdit->addAction(m_editSelectAll);
	connect(m_editSelectAll, &QAction::triggered, this, &menubar_t::selectAllTriggered);

	m_editPaste->setText("Paste");
	m_editPaste->setShortcut(QKeySequence::Paste);
	m_editPaste->setStatusTip("Paste");
	m_menuEdit->addAction(m_editPaste);
	connect(m_editPaste, &QAction::triggered, this, &menubar_t::pasteTriggered);

	m_helpHelp->setText("Help");
	m_helpHelp->setShortcut(QKeySequence::HelpContents);
	m_helpHelp->setStatusTip("Help!");
	m_menuHelp->addAction(m_helpHelp);
	connect(m_helpHelp, &QAction::triggered, this, &menubar_t::helpTriggered);

	m_helpAbout->setText("About");
	m_helpAbout->setShortcut(Qt::Key_A|Qt::Modifier::CTRL);
	m_helpAbout->setStatusTip("About WINHTTPD");
	m_menuHelp->addAction(m_helpAbout);
	connect(m_helpAbout, &QAction::triggered, this, &menubar_t::aboutTriggered);

	m_menuSettings->setText("Settings");
	m_menuSettings->setShortcut(Qt::Key_I | Qt::Modifier::CTRL);
	m_menuSettings->setStatusTip("Configure the WINHTTPD GUI");
	m_menuFile->addAction(m_menuSettings);
	connect(m_menuSettings, &QAction::triggered, this, &menubar_t::settingsTriggered);

	m_startService->setText("Start Service");
	m_startService->setShortcut(Qt::Modifier::CTRL|Qt::Key_S);
	m_startService->setStatusTip("Start Service");
	m_menuFileService->addAction(m_startService);
	connect(m_startService, &QAction::triggered, this, &menubar_t::startServiceTriggered);

	m_stopService->setText("Stop Service");
	m_stopService->setShortcut(Qt::Modifier::CTRL | Qt::Key_E);
	m_stopService->setStatusTip("Stop Service");
	m_menuFileService->addAction(m_stopService);
	connect(m_stopService, &QAction::triggered, this, &menubar_t::stopServiceTriggered);

	m_pauseService->setText("Pause Service");
	m_pauseService->setShortcut(Qt::Modifier::CTRL | Qt::Key_P);
	m_pauseService->setStatusTip("Pause Service");
	m_menuFileService->addAction(m_pauseService);
	connect(m_pauseService, &QAction::triggered, this, &menubar_t::pauseServiceTriggered);

	m_resumeService->setText("Resume Service");
	m_resumeService->setShortcut(Qt::Modifier::CTRL | Qt::Key_M);
	m_resumeService->setStatusTip("Resume Service");
	m_menuFileService->addAction(m_resumeService);
	connect(m_pauseService, &QAction::triggered, this, &menubar_t::resumeServiceTriggered);

	m_menuFile->addSeparator();
	m_menuExit->setText("Exit");
	m_menuExit->setShortcut(Qt::Modifier::CTRL | Qt::Key_X); //QKeySequence::Quit);
	m_menuExit->setStatusTip("Exit the WINHTTPD GUI");
	m_menuFile->addAction(m_menuExit);
	connect(m_menuExit, &QAction::triggered, this, &menubar_t::exitTriggered);

	return;
}

menubar_t::~menubar_t(void)
{
	return;
}

void 
menubar_t::setServiceState(QString& s)
{
	if (true == s.startsWith("start", Qt::CaseInsensitive)) {
		m_startService->setDisabled(true);
		m_stopService->setEnabled(true);
		m_pauseService->setEnabled(true);
		m_resumeService->setDisabled(true);

	} else if (true == s.startsWith("sto", Qt::CaseInsensitive)) {
		m_startService->setEnabled(true);
		m_stopService->setDisabled(true);
		m_pauseService->setDisabled(true);
		m_resumeService->setDisabled(true);
	
	} else if ( true == s.startsWith("pau", Qt::CaseInsensitive)) {
		m_startService->setDisabled(true);
		m_stopService->setEnabled(true);
		m_pauseService->setDisabled(true);
		m_resumeService->setEnabled(true);
	
	} else if (true == s.startsWith("res", Qt::CaseInsensitive)) {
		m_startService->setDisabled(true);
		m_stopService->setEnabled(true);
		m_pauseService->setDisabled(true);
		m_resumeService->setDisabled(true);
	
	} else {
		m_startService->setEnabled(true);
		m_stopService->setEnabled(true);
		m_pauseService->setEnabled(true);
		m_resumeService->setEnabled(true);
	}

	return;
}


void 
menubar_t::settingsTriggered(void)
{
	return;
}

void 
menubar_t::exitTriggered(void)
{
	QCoreApplication::quit();
	return;
}

void 
menubar_t::cutTriggered(void)
{
	emit cutRaised();
	return;
}

void 
menubar_t::copyTriggered(void)
{
	emit copyRaised();
	return;
}

void 
menubar_t::pasteTriggered(void)
{
	emit pasteRaised();
	return;
}

void 
menubar_t::selectAllTriggered(void)
{
	emit selectAllRaised();
	return;
}

void 
menubar_t::helpTriggered(void)
{
	emit helpRaised();
	return;
}

void 
menubar_t::aboutTriggered(void)
{
	emit aboutRaised();

	return;
}

void
menubar_t::startServiceTriggered(void)
{
	emit startServiceRaised();
	return;
}

void
menubar_t::stopServiceTriggered(void)
{
	emit stopServiceRaised();
	return;
}

void
menubar_t::pauseServiceTriggered(void)
{
	emit pauseServiceRaised();
	return;
}

void
menubar_t::resumeServiceTriggered(void)
{
	emit resumeServiceRaised();
	return;
}