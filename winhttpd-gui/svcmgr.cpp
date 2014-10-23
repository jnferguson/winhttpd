#include "svcmgr.h"

svcMgr_t::svcMgr_t(const QString& title, QWidget* p) : QDialog(p), m_controller("WINHTTPD-Controller"), m_worker("WINHTTPD-Worker")
{
	m_layout	= new QVBoxLayout(this);
	m_label		= new QLabel(this);
	m_cancel	= new QPushButton(this);
	m_loadLabel	= new QLabel(this);
	m_loading	= new QMovie(":/loading.gif");

	resize(250, 150);
	setLayout(m_layout);

	m_label->setAlignment(Qt::AlignCenter|Qt::AlignHCenter|Qt::AlignVCenter);
	m_label->setTextInteractionFlags(Qt::NoTextInteraction);
	
	m_loadLabel->setMovie(m_loading);
	m_loadLabel->setAlignment(Qt::AlignCenter|Qt::AlignHCenter|Qt::AlignVCenter);
	m_loadLabel->setTextInteractionFlags(Qt::NoTextInteraction);

	m_layout->addWidget(m_loadLabel);
	m_layout->addWidget(m_label);
	m_layout->addWidget(m_cancel);
	m_loading->start();

	setWindowTitle(title);
	
	m_label->setToolTip(title);
	m_label->setStatusTip(title);
	m_label->setWhatsThis(title);
	m_label->setText(title);

	m_cancel->setText("Cancel Operation");
	m_cancel->setToolTip("Cancel Operation");
	m_cancel->setStatusTip("Cancel Operation");
	m_cancel->setWhatsThis("Cancel Operation");

	connect(m_cancel, &QPushButton::clicked, this, &QDialog::close);
	connect(this, &svcMgr_t::started, this, &svcMgr_t::start);
	connect(this, &svcMgr_t::stopped, this, &svcMgr_t::stop);
	connect(this, &svcMgr_t::paused, this, &svcMgr_t::pause);
	connect(this, &svcMgr_t::resumed, this, &svcMgr_t::resume);
	connect(this, &svcMgr_t::failed, this, &QDialog::close); 
	connect(this, &svcMgr_t::success, this, &QDialog::close); 
	connect(this, &svcMgr_t::updateStatus, this, &svcMgr_t::updateLabel);

	return;
}

svcMgr_t::~svcMgr_t(void)
{
	return;
}

bool
svcMgr_t::startService(const QString& n, bool silent)
{
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	LPCWSTR					av[2]	= { nullptr, nullptr };
	SERVICE_STATUS_PROCESS	stat	= {0};
	DWORD					len		= sizeof(stat);
	ULONGLONG				tick	= 0;

	av[0] = n.toStdWString().c_str();

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE|SC_MANAGER_LOCK);

	if (nullptr == mgr) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to open the Windows Service Manager");

		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), STANDARD_RIGHTS_EXECUTE|SC_MANAGER_CONNECT|SC_MANAGER_LOCK|SERVICE_QUERY_STATUS|SERVICE_START);

	if (nullptr == svc) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to open the service " + n + ":");

		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to query the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}


	if (SERVICE_RUNNING == stat.dwCurrentState) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return true;
	}

	if (	SERVICE_STOP_PENDING == stat.dwCurrentState || 
			SERVICE_PAUSE_PENDING == stat.dwCurrentState || 
			SERVICE_START_PENDING == stat.dwCurrentState) {

		if (! silent)
			util::error("Error in statusTab_t::startService()", "The service " + n + " is pending a previous control operation", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::StartService(svc, 1, &av[0])) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to start the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	tick = ::GetTickCount64();

	while (SERVICE_START_PENDING == stat.dwCurrentState) {
		qApp->processEvents();
		QThread::msleep(1);
		qApp->processEvents();	

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::startService()", "Error while attempting to query the service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (::GetTickCount() - tick > stat.dwWaitHint) {
			if (! silent)
				util::error("Error in statusTab_t::startService()", "Timed out while waiting for service " + n + " to start", false);

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

	}

	if (SERVICE_RUNNING != stat.dwCurrentState) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Timed out while waiting for service " + n + "to start", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;		
}

bool 
svcMgr_t::stopService(const QString& n, bool silent)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					len		= 0;
	ULONGLONG				tick	= 0;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		if (! silent)
			util::error("Error in statusTab_t::stopService()", "Error while attempting to open the Windows Service Manager");

		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		if (! silent)
			util::error("Error in statusTab_t::stopService()", "Error while attempting to open the service " + n + ":");

		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		if (! silent)
			util::error("Error in statusTab_t::stopService()", "Error while attempting to query the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED == stat.dwCurrentState) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return true;
	}

	if (	SERVICE_STOP_PENDING == stat.dwCurrentState || 
			SERVICE_PAUSE_PENDING == stat.dwCurrentState || 
			SERVICE_START_PENDING == stat.dwCurrentState) {

		if (! silent)
			util::error("Error in statusTab_t::stopService()", "The service " + n + " is pending a previous control operation", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&stat)) {
		if (! silent)
			util::error("Error in statusTab_t::stopService()", "ASDF: Error while attempting to control the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	tick = ::GetTickCount64();

	while (SERVICE_STOPPED != stat.dwCurrentState) {
		qApp->processEvents();
		QThread::msleep(1);
		qApp->processEvents();	

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::stopService()", "Error while attempting to query the service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (SERVICE_STOPPED == stat.dwCurrentState) 
			break;

		if (::GetTickCount() - tick > stat.dwWaitHint) {
			if (! silent)
				util::error("Error in statusTab_t::stopService()", "Timed out while attempting to stop service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

bool 
svcMgr_t::pauseService(const QString& n, bool silent)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					len		= 0;
	ULONGLONG				tick	= 0;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "Error while attempting to open the Windows Service Manager");

		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "Error while attempting to open the service " + n + ":");

		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "Error while attempting to query the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_RUNNING != stat.dwCurrentState) {
		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "Attempted to pause service " + n + " however, it is not currently running", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);		
		return false;
	}

	if (	SERVICE_STOP_PENDING == stat.dwCurrentState || 
			SERVICE_PAUSE_PENDING == stat.dwCurrentState || 
			SERVICE_START_PENDING == stat.dwCurrentState) {

		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "The service " + n + " is pending a previous control operation", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_PAUSE, (LPSERVICE_STATUS)&stat)) {
		if (! silent)
			util::error("Error in statusTab_t::pauseService()", "Error while attempting to control the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	tick = ::GetTickCount64();

	while (SERVICE_PAUSED != stat.dwCurrentState) {
		qApp->processEvents();
		QThread::msleep(1);
		qApp->processEvents();	

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::pauseService()", "Error while attempting to query the service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (SERVICE_PAUSED == stat.dwCurrentState) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return true;			
		}

		if (::GetTickCount() - tick > stat.dwWaitHint) {
			if (! silent)
				util::error("Error in statusTab_t::pauseService()", "Timed out while attempting to pause service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

bool 
svcMgr_t::resumeService(const QString& n, bool silent)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					len		= 0;
	ULONGLONG				tick	= 0;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "Error while attempting to open the Windows Service Manager");

		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "Error while attempting to open the service " + n + ":");

		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "Error while attempting to query the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_PAUSED != stat.dwCurrentState) {
		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "Attempted to resume service " + n + " however, it is not currently paused", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (	SERVICE_STOP_PENDING == stat.dwCurrentState || 
			SERVICE_PAUSE_PENDING == stat.dwCurrentState || 
			SERVICE_START_PENDING == stat.dwCurrentState) {

		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "The service " + n + " is pending a previous control operation", false);

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_CONTINUE, (LPSERVICE_STATUS)&stat)) {
		if (! silent)
			util::error("Error in statusTab_t::resumeService()", "Error while attempting to control the service " + n + ":");

		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	tick = ::GetTickCount64();

	while (SERVICE_CONTINUE_PENDING == stat.dwCurrentState) {
		qApp->processEvents();
		QThread::msleep(1);
		qApp->processEvents();	

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::resumeService()", "Error while attempting to query the service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (SERVICE_RUNNING == stat.dwCurrentState) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return true;			
		}

		if (::GetTickCount() - tick > stat.dwWaitHint) {
			if (! silent)
				util::error("Error in statusTab_t::resumeService()", "Timed out while attempting to resume service " + n + ":");

			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

void 
svcMgr_t::start(void)
{
	bool		status	= false;
	std::size_t	count	= 0;

	for (count = 0; count < 5120; count++) {
		qApp->processEvents();

		if (false == status) {
			emit updateStatus("Starting service " + m_controller + "...");

			if (true == startService(m_controller, true))
				status = true;
		} else {
			emit updateStatus("Starting service " + m_worker + "...");

			if (true == startService(m_worker, true)) {
				emit success();
				return;
			}
		}

		QThread::msleep(1);
	}

	if (true == status) {
		if (true == startService(m_worker)) {
			emit success();
			return;
		}

		stopService(m_controller);

	} else {
		if (true == startService(m_controller)) {
			if (true == startService(m_worker)) {
				emit success();
				return;
			}

			stopService(m_controller);
		}
	}
	
	emit failed();
	return;
}

void 
svcMgr_t::stop(void)
{
	bool		status  = false;
	std::size_t	count	= 0;

	for (count = 0; count < 5120; count++) {
		qApp->processEvents();

		if (false == status) {
			emit updateStatus("Stopping service " + m_worker + "...");

			if (true == stopService(m_worker, true))
				status = true;
		} else {
			emit updateStatus("Stopping service " + m_controller + "...");

			if (true == stopService(m_controller, true)) {
				emit success();
				return;
			}
		}

		QThread::msleep(1);
	}

	if (true == status) {
		if (true == stopService(m_controller)) {
			emit success();
			return;
		}

		startService(m_worker);

	} else {
		if (true == stopService(m_worker)) {
			if (true == stopService(m_controller)) {
				emit success();
				return;
			}

			startService(m_worker);
		}
	}
	
	emit failed();
	return;
}

void 
svcMgr_t::pause(void)
{
	bool		status  = false;
	std::size_t	count	= 0;

	for (count = 0; count < 5120; count++) {
		qApp->processEvents();

		if (false == status) {
			emit updateStatus("Pausing service " + m_worker + "...");

			if (true == pauseService(m_worker, true))
				status = true;
		} else {
			emit updateStatus("Pausing service " + m_controller + "...");

			if (true == pauseService(m_controller, true)) {
				emit success();
				return;
			}
		}

		QThread::msleep(1);
	}

	if (true == status) {
		if (true == pauseService(m_controller)) {
			emit success();
			return;
		}

		resumeService(m_worker);

	} else {
		if (true == pauseService(m_worker)) {
			if (true == pauseService(m_controller)) {
				emit success();
				return;
			}

			resumeService(m_worker);
		}
	}
	
	emit failed();
	return;
}

void 
svcMgr_t::resume(void)
{
	bool		status	= false;
	std::size_t	count	= 0;

	for (count = 0; count < 5120; count++) {
		qApp->processEvents();

		if (false == status) {
			emit updateStatus("Resuming service " + m_controller + "...");

			if (true == resumeService(m_controller, true))
				status = true;
		} else {
			emit updateStatus("Resuming service " + m_worker + "...");

			if (true == resumeService(m_worker, true)) {
				emit success();
				return;
			}
		}

		QThread::msleep(1);
	}

	if (true == status) {
		if (true == resumeService(m_worker)) {
			emit success();
			return;
		}

		pauseService(m_controller);

	} else {
		if (true == resumeService(m_controller)) {
			if (true == resumeService(m_worker)) {
				emit success();
				return;
			}

			pauseService(m_controller);
		}
	}
	
	emit failed();
	return;
}

void 
svcMgr_t::updateLabel(const QString& t)
{
	m_label->setText(t);
	m_label->setToolTip(t);
	m_label->setStatusTip(t);
	m_label->setWhatsThis(t);
	qApp->processEvents();
	return;
}

void 
svcMgr_t::start_service(void)
{
	show();
	emit started();
	return;
}

void 
svcMgr_t::stop_service(void)
{
	show();
	emit stopped();
	return;
}

void 
svcMgr_t::pause_service(void)
{
	show();
	emit paused();
	return;
}

void 
svcMgr_t::resume_service(void)
{
	show();
	emit resumed();
	return;
}
