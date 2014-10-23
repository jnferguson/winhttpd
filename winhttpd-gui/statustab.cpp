#include "statustab.h"

statusTab_t::statusTab_t(QWidget* p) : QWidget(p)
{
	m_timer		= new QTimer(this);
	m_layout	= new QVBoxLayout(this);
	m_statBox	= new QGroupBox(this);
	m_logBox	= new QGroupBox(this);
	m_hlayout	= new QHBoxLayout(m_statBox);
	m_light		= new statusLight_t(m_statBox); 
	m_combo		= new QComboBox(m_statBox);
	m_vlayout	= new QVBoxLayout(m_logBox);
	m_log		= new logWidget_t(m_logBox);

	setParent(p);

	m_statBox->setToolTip("Displays the current status of the service...");
	m_statBox->setWhatsThis("Displays the current status of the service...");

	m_logBox->setToolTip("Displays various service error, warning and informational messages...");
	m_logBox->setWhatsThis("Displays various service error, warning and informational messages...");

	m_combo->setToolTip("Change the status of the service");
	m_combo->setWhatsThis("Change the status of the service");

	m_light->setToolTip("A visual indicator of the service status for both the worker and controller processes");
	m_light->setWhatsThis("A visual indicator of the service status for both the worker and controller processes");

	m_timer->setInterval(1500);
	connect(m_timer, &QTimer::timeout, this, &statusTab_t::updateLightStatus);

	m_statBox->setTitle("Service Status");
	m_logBox->setTitle("System Log");

	m_hlayout->setAlignment(Qt::AlignVCenter);
	m_hlayout->addWidget(m_light);
	m_hlayout->addWidget(m_combo);
	
	m_vlayout->addWidget(m_log);
	
	m_layout->addWidget(m_statBox);
	m_layout->addWidget(m_logBox);

	connect(m_combo, SIGNAL(activated(const QString&)), this, SLOT(comboActivated(const QString&)));
	connect(this, &statusTab_t::setStatus, m_light, &statusLight_t::setState);
	updateLightStatus();
	m_timer->start();
	
	return;
}

statusTab_t::~statusTab_t(void)
{
	return;
}


void
statusTab_t::updateLightStatus(void)
{
	DWORD none			= ~(SERVICE_RUNNING|SERVICE_STOPPED|SERVICE_PAUSED);
	DWORD controller	= getServiceStatus("WINHTTPD-Controller");
	DWORD worker		= getServiceStatus("WINHTTPD-Worker");

	if (SERVICE_RUNNING == controller && SERVICE_RUNNING == worker) {
		m_state = statusLight_t::SERVICES_RUNNING;
		emit setStatus(m_state);
	} else if (SERVICE_RUNNING == controller && SERVICE_RUNNING != worker) {
		m_state = statusLight_t::CONTROLLER_RUNNING;
		emit setStatus(m_state);
	} else if (SERVICE_RUNNING != controller && SERVICE_RUNNING == worker) {
		m_state = statusLight_t::WORKER_RUNNING;
		emit setStatus(m_state);
	} else if (SERVICE_STOPPED == controller && SERVICE_STOPPED == worker) {
		m_state = statusLight_t::SERVICES_STOPPED;
		emit setStatus(m_state);
	} else if (SERVICE_STOPPED == controller && SERVICE_STOPPED != worker) {
		m_state = statusLight_t::CONTROLLER_STOPPED;
		emit setStatus(m_state);
	} else if (SERVICE_STOPPED != controller && SERVICE_STOPPED == worker) {
		m_state = statusLight_t::WORKER_STOPPED;
		emit setStatus(m_state);
	} else if (SERVICE_PAUSED == controller && SERVICE_PAUSED == worker) {
		m_state = statusLight_t::SERVICES_PAUSED;
		emit setStatus(m_state);
	} else if (SERVICE_PAUSED == controller && SERVICE_PAUSED != worker) {
		m_state = statusLight_t::CONTROLLER_PAUSED;
		emit setStatus(m_state);
	} else if (SERVICE_PAUSED != controller && SERVICE_PAUSED == worker) {
		m_state = statusLight_t::WORKER_PAUSED;
		emit setStatus(m_state);
	} else if (none == controller && none == worker) {
		m_state = statusLight_t::SERVICES_UNKNOWN;
		emit setStatus(m_state);
	} else if (none == controller && none != worker) {
		m_state = statusLight_t::CONTROLLER_UNKNOWN;
		emit setStatus(m_state);
	} else if (none != controller && none == worker) {
		m_state = statusLight_t::WORKER_UNKNOWN;
		emit setStatus(m_state);
	} else {
		m_state = statusLight_t::SERVICES_UNKNOWN;
		emit setStatus(m_state);
	}

	initComboBox();
	return;
}

void
statusTab_t::initComboBox(void)
{
	QStringList		comboItems;
	std::size_t 	runState(statusLight_t::SERVICES_RUNNING | statusLight_t::CONTROLLER_RUNNING | statusLight_t::WORKER_RUNNING);
	std::size_t		stopState(statusLight_t::SERVICES_STOPPED | statusLight_t::CONTROLLER_STOPPED | statusLight_t::WORKER_STOPPED);
	std::size_t		pauseState(statusLight_t::SERVICES_PAUSED | statusLight_t::CONTROLLER_PAUSED | statusLight_t::WORKER_PAUSED);
	std::size_t		state(static_cast<std::size_t>(m_state));

	m_combo->clear();

	if ((state & runState) == state) {
		comboItems << "Stop" << "Pause";
		emit updateStatus(QString("Started"));
	
	} else if ((state & pauseState) == state) {
		comboItems << "Resume" << "Stop";
		emit updateStatus(QString("Paused"));
	
	} else if ((state & stopState) == state) {
		comboItems << "Start";
		emit updateStatus(QString("Stopped"));

	} else {
		emit updateStatus(QString("Unknown"));
		comboItems << "Start" << "Stop" << "Pause" << "Resume";
	}

	m_combo->insertItems(0, comboItems);

	return;
}

bool 
statusTab_t::startServices(void)
{
	svcMgr_t s("Starting Service...");
	s.start_service();
	emit updateStatus(QString("Started"));
	return true;
}

bool 
statusTab_t::stopServices(void)
{
	svcMgr_t s("Stopping Service...");
	s.stop_service();
	emit updateStatus(QString("Stopped"));
	return true;
}

bool 
statusTab_t::pauseServices(void)
{
	svcMgr_t s("Pausing Service...");
	s.pause_service();
	emit updateStatus(QString("Paused"));
	return true;
}

bool 
statusTab_t::resumeServices(void)
{
	svcMgr_t s("Resuming Service...");
	s.resume_service();
	emit updateStatus(QString("Resumed"));
	return true;
}

void 
statusTab_t::start_svc(void)
{
	(void)startServices();
	updateLightStatus();
	return;
}

void 
statusTab_t::stop_svc(void)
{
	(void)stopServices();
	updateLightStatus();
	return;
}

void 
statusTab_t::pause_svc(void)
{
	(void)pauseServices();
	updateLightStatus();
	return;
}
void 
statusTab_t::resume_svc(void)
{
	(void)resumeServices();
	updateLightStatus();
	return;
}

void 
statusTab_t::comboActivated(const QString& n)
{
	if (! n.compare("start", Qt::CaseInsensitive)) {
		startServices();	
		updateLightStatus();

	} else if (! n.compare("stop", Qt::CaseInsensitive)) {
		stopServices();
		updateLightStatus();

	} else if (! n.compare("pause", Qt::CaseInsensitive)) {
		pauseServices();
		updateLightStatus();

	} else if (! n.compare("resume", Qt::CaseInsensitive)) {
		resumeServices();
		updateLightStatus();
	}

	return;
}


DWORD 
statusTab_t::getServiceStatus(QString n)
{
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	SERVICE_STATUS_PROCESS	stat	= {0};
	DWORD					len		= sizeof(stat);

	::SetLastError(NO_ERROR);

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE|SC_MANAGER_LOCK);

	if (nullptr == mgr) {
		util::error("Error", "Error while attempting to open the Windows Service Manager");
		return 0;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), STANDARD_RIGHTS_EXECUTE|SC_MANAGER_CONNECT|SC_MANAGER_LOCK|SERVICE_QUERY_STATUS|SERVICE_START);

	if (nullptr == svc) {
		util::error("Error", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return 0;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		util::error("Error", "Error while attempting to query the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return 0;
	}


	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return stat.dwCurrentState;
}

bool 
statusTab_t::startService(QString n, bool silent)
{
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	LPCWSTR					av[2]	= { nullptr, nullptr };
	SERVICE_STATUS_PROCESS	stat	= {0};
	DWORD					len		= sizeof(stat);
	DWORD					stick	= 0;
	DWORD					ocheck	= 0;

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

	if (SERVICE_STOPPED != stat.dwCurrentState && SERVICE_STOP_PENDING != stat.dwCurrentState 
		&& SERVICE_PAUSED != stat.dwCurrentState && SERVICE_PAUSE_PENDING != stat.dwCurrentState) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return true;
	}

	stick	= ::GetTickCount();
	ocheck	= stat.dwCheckPoint;

	while (SERVICE_STOP_PENDING == stat.dwCurrentState) {
		DWORD wtime = stat.dwWaitHint / 10;

		if (1000 > wtime) 
			wtime = 1000;
		else if (10000 <= wtime)
			wtime = 10000;

		::Sleep(wtime);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::startService()", "Error while attempting to query the service " + n + ":");
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (stat.dwCheckPoint > ocheck) {
			stick	= ::GetTickCount();
			ocheck	= stat.dwCheckPoint;
		} else {
			if (::GetTickCount() - stick > stat.dwWaitHint) {
				if (! silent)
					util::error("Error in statusTab_t::startService()", "Timed out while waiting for service " + n + " to start", false);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				return false;
			}
		}
	}

	if (FALSE == ::StartService(svc, 1, &av[0])) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to start the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Error while attempting to query the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	stick	= ::GetTickCount();
	ocheck	= stat.dwCheckPoint;

	while (SERVICE_START_PENDING == stat.dwCurrentState) {
		DWORD wtime = stat.dwWaitHint / 10;

		if (1000 > wtime) 
			wtime = 1000;
		else if (10000 <= wtime)
			wtime = 10000;

		::Sleep(wtime);	

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			if (! silent)
				util::error("Error in statusTab_t::startService()", "Error while attempting to query the service " + n + ":");
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (stat.dwCheckPoint > ocheck) {
			stick	= ::GetTickCount();
			ocheck	= stat.dwCheckPoint;
		} else {
			if (::GetTickCount() - stick > stat.dwWaitHint) {
				break;
			}
		}
	}


	if (SERVICE_RUNNING != stat.dwCurrentState) {
		if (! silent)
			util::error("Error in statusTab_t::startService()", "Timed out while waiting for service " + n + " to start", false);
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

bool 
statusTab_t::stopService(QString n)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					stime	= ::GetTickCount();
	DWORD					len		= 0;
	DWORD					wtime	= 0;
	DWORD					tout	= 30000;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		util::error("Error in statusTab_t::stopService()", "Error while attempting to open the Windows Service Manager");
		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		util::error("Error in statusTab_t::stopService()", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		util::error("Error in statusTab_t::stopService()", "Error while attempting to query the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED == stat.dwCurrentState) 
		return true;


	while (SERVICE_STOP_PENDING == stat.dwCurrentState) {

		if (1000 > wtime) 
			wtime = 1000;
		else if (10000 <= wtime)
			wtime = 10000;

		::Sleep(wtime);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			util::error("Error in statusTab_t::stopService()", "Error while attempting to query the service " + n + ":");
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (SERVICE_STOPPED == stat.dwCurrentState) 
			return true;			

		if (::GetTickCount() - stime > tout) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	if (false == this->stopDependencies(n, mgr, svc)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}


	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&stat)) {
		util::error("Error in statusTab_t::stopService()", "Error while attempting to control the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED != stat.dwCurrentState)  
		::Sleep(1000);

	while (SERVICE_STOPPED != stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
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

		if (::GetTickCount() - stime > tout) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			break;
		}
	}

	return false;
}

bool
statusTab_t::stopDependencies(QString n, SC_HANDLE m, SC_HANDLE h)
{
	DWORD					i		= 0;
    DWORD					needed	= 0;
    DWORD					count	= 0;
    LPENUM_SERVICE_STATUS   deps	= nullptr;
	ENUM_SERVICE_STATUS     ess		= {0};
    SC_HANDLE               deph	= nullptr;
	SERVICE_STATUS_PROCESS  ssp		= {0};
	DWORD					start	= ::GetTickCount();
    DWORD					tout	= 30000;
	DWORD					e		= 0;

	if (nullptr == m || nullptr == h) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		util::error("Error in statusTab_t::stopDependencies()", "Error: Invalid parameters specified", false);
		return false;
	}

	if (TRUE == ::EnumDependentServices(h, SERVICE_ACTIVE, deps, 0, &needed, &count)) 
		return true;

	if (ERROR_MORE_DATA != ::GetLastError()) {
		util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to allocate memory");
		return false;
	}

	if (FALSE == ::EnumDependentServices(h, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		::HeapFree(::GetProcessHeap(), 0, deps);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(m, ess.lpServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to open the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_STOP, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to stop the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_STOPPED != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				util::error("Error in statusTab_t::stopDependencies()", "Error while attempting to query the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
				::CloseServiceHandle(deph);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_STOPPED == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				util::error("Error in statusTab_t::stopDependencies()", "Timed out while waiting for service dependency '" + QString::fromWCharArray(ess.lpServiceName) + "' to stop");
				::CloseServiceHandle(deph);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}
		}
	}

	return true;
}

bool 
statusTab_t::pauseService(QString n)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					stime	= ::GetTickCount();
	DWORD					len		= 0;
	DWORD					wtime	= 0;
	DWORD					tout	= 30000;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		util::error("Error in statusTab_t::pauseService()", "Error while attempting to open the Windows Service Manager");
		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		util::error("Error in statusTab_t::pauseService()", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		util::error("Error in statusTab_t::pauseService()", "Error while attempting to query the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_RUNNING != stat.dwCurrentState) 
		return false;

	while (SERVICE_START_PENDING == stat.dwCurrentState) {

		if (1000 > wtime) 
			wtime = 1000;
		else if (10000 <= wtime)
			wtime = 10000;

		::Sleep(wtime);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			util::error("Error in statusTab_t::pauseService()", "Error while attempting to query the service " + n + ":");
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (SERVICE_RUNNING == stat.dwCurrentState)
			break;			

		if (::GetTickCount() - stime > tout) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_PAUSE, (LPSERVICE_STATUS)&stat)) {
		util::error("Error in statusTab_t::pauseService()", "Error while attempting to control the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_PAUSED != stat.dwCurrentState) 
		::Sleep(1000);

	while (SERVICE_PAUSED != stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
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

		if (::GetTickCount() - stime > tout) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	return true;
}

bool 
statusTab_t::resumeService(QString n)
{
	SERVICE_STATUS_PROCESS  stat	= {0};
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	DWORD					stime	= ::GetTickCount();
	DWORD					len		= 0;
	DWORD					wtime	= 0;
	DWORD					tout	= 30000;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		util::error("Error in statusTab_t::resumeService()", "Error while attempting to open the Windows Service Manager");
		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		util::error("Error in statusTab_t::resumeService()", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		util::error("Error in statusTab_t::resumeService()", "Error while attempting to query the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_PAUSED != stat.dwCurrentState) 
		return false;

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_CONTINUE, (LPSERVICE_STATUS)&stat)) {
		util::error("Error in statusTab_t::resumeService()", "Error while attempting to control the service " + n + ":");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_RUNNING != stat.dwCurrentState) 
		::Sleep(1000);

	while (SERVICE_CONTINUE_PENDING == stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
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

		if (::GetTickCount() - stime > tout) {
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}
	}

	return true;
}

bool 
statusTab_t::pauseDependencies(QString n)
{
	DWORD					i		= 0;
    DWORD					needed	= 0;
    DWORD					count	= 0;
    LPENUM_SERVICE_STATUS   deps	= nullptr;
	ENUM_SERVICE_STATUS     ess		= {0};
    SC_HANDLE               deph	= nullptr;
	SERVICE_STATUS_PROCESS  ssp		= {0};
	DWORD					start	= ::GetTickCount();
    DWORD					tout	= 30000;
	DWORD					e		= 0;
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;


	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to open the Windows Service Manager");
		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (TRUE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, 0, &needed, &count)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);	
		return true;
	}

	if (ERROR_MORE_DATA != ::GetLastError()) {
		util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		util::error("Error in statusTab_t::pauseDependencies()", "Error while allocating memory");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		::HeapFree(::GetProcessHeap(), 0, deps);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(mgr, ess.lpServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to open the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_CONTINUE, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to stop the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_RUNNING != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				util::error("Error in statusTab_t::pauseDependencies()", "Error while attempting to query the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_RUNNING == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				util::error("Error in statusTab_t::pauseDependencies()", "Timed out while waiting for service dependency '" + QString::fromWCharArray(ess.lpServiceName) + "' to stop");
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}
		}
	}

	return true;
}

bool
statusTab_t::resumeDependencies(QString n)
{
	DWORD					i		= 0;
    DWORD					needed	= 0;
    DWORD					count	= 0;
    LPENUM_SERVICE_STATUS   deps	= nullptr;
	ENUM_SERVICE_STATUS     ess		= {0};
    SC_HANDLE               deph	= nullptr;
	SERVICE_STATUS_PROCESS  ssp		= {0};
	DWORD					start	= ::GetTickCount();
    DWORD					tout	= 30000;
	DWORD					e		= 0;
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;

	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (nullptr == mgr) {
		util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to open the Windows Service Manager");
		return false;
	}

	svc = ::OpenService(mgr, n.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to open the service " + n + ":");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (TRUE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, 0, &needed, &count)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);	
		return true;
	}

	if (ERROR_MORE_DATA != ::GetLastError()) {
		util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		util::error("Error in statusTab_t::resumeDependencies()", "Error while allocating memory");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to enumerate service " + n + "'s dependencies");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		::HeapFree(::GetProcessHeap(), 0, deps);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(mgr, ess.lpServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to open the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_PAUSE, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to stop the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_PAUSED != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				util::error("Error in statusTab_t::resumeDependencies()", "Error while attempting to query the service dependency: " + QString::fromWCharArray(ess.lpServiceName));
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_PAUSED == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				util::error("Error in statusTab_t::resumeDependencies()", "Timed out while waiting for service dependency '" + QString::fromWCharArray(ess.lpServiceName) + "' to stop");
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}
		}
	}

	return true;
}
