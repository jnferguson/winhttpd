#include "mwindow.h"

mwindow_t::mwindow_t(QWidget* p) : QMainWindow(p)
{
	QPixmap	icon(":/icons/Resources/winhttpd.ico");

	m_menu				= new menubar_t(this); 
	m_status			= new QStatusBar(this);
	m_central			= new QWidget(this);

	m_layout			= new QHBoxLayout(m_central);
	m_tabs				= new QTabWidget(m_central);

	m_statusTab			= new statusTab_t(m_tabs); 
	m_connectlogTab		= new connectionTab_t(m_tabs);
	m_configTab			= new configTab_t(m_tabs);
	m_userTab			= new userTab_t(m_tabs); 
	m_hostTab			= new hostTab_t(m_tabs);

	setWindowIcon(icon);

	resize(800, 600);
	setCentralWidget(m_central);
	setMenuBar(m_menu);
	setStatusBar(m_status);
	setWindowTitle("WINHTTPD");

	m_tabs->addTab(m_statusTab, "Status");
	m_tabs->addTab(m_connectlogTab, "Connection Log");
	m_tabs->addTab(m_userTab, "Users");
	m_tabs->addTab(m_hostTab, "Hosts");
	m_tabs->addTab(m_configTab,"Configuration");


	connect(m_statusTab, &statusTab_t::updateStatus, m_menu, &menubar_t::setServiceState);

	connect(m_menu, &menubar_t::startServiceRaised, m_statusTab, &statusTab_t::start_svc);
	connect(m_menu, &menubar_t::stopServiceRaised, m_statusTab, &statusTab_t::stop_svc);
	connect(m_menu, &menubar_t::pauseServiceRaised, m_statusTab, &statusTab_t::pause_svc);
	connect(m_menu, &menubar_t::resumeServiceRaised, m_statusTab, &statusTab_t::resume_svc);

	m_layout->addWidget(m_tabs);
	m_tabs->setCurrentIndex(0);
	return;
}

mwindow_t::~mwindow_t(void)
{
	return;
}