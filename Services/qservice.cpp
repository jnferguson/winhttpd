#include "qservice.h"

qservice_t* qservice_t::m_instance = nullptr;

qservice_t::qservice_t(QString n, QString d, QString e, QString l, QObject* p) 
	: QObject(p),
	m_svcname(n),
	m_dispname(d),
	m_description(e),
	m_path(l),
	m_log(new eventLog_t("Application", n)),
	m_handle(nullptr),
	m_checkpoint(0),
	m_access(SERVICE_ALL_ACCESS),
	m_type(SERVICE_WIN32_OWN_PROCESS),
	m_start(SERVICE_AUTO_START),
	m_error(SERVICE_ERROR_NORMAL),
	m_loadgroup(nullptr),
	m_tagid(0),
	m_deps(""),
	m_user(""),
	m_pass("")
{

	if (nullptr != qservice_t::m_instance) {
		m_log->error("Attempt to instantiate singleton object a second time", eventlog_cat_t::CAT_SERVICE, false);
		throw std::runtime_error("Attempt to instantiate singleton object a second time");
	}

	if (nullptr == d)
		m_dispname = n;

	qservice_t::m_instance = this;

	m_status.dwServiceType		= m_type;
	m_status.dwWin32ExitCode	= 0;

	return;
}

qservice_t::~qservice_t(void)
{
	return;
}


VOID WINAPI 
qservice_t::ServiceMain(DWORD ac, LPTSTR* av)
{
	qservice_t& i = qservice_t::instance();

	i.start(ac, av);
	return;
}

void
qservice_t::start(DWORD ac, LPTSTR* av)
{
	LPHANDLER_FUNCTION_EX scf = &qservice_t::ServiceCtrl;

	if (nullptr == scf) {
		m_log->error("Invalid qservice_t::ServiceCtrl pointer (null)", eventlog_cat_t::CAT_SERVICE, false);
		throw std::runtime_error("Invalid qservice_t::ServiceCtrl pointer (null)");
	}

	m_handle = ::RegisterServiceCtrlHandlerEx(m_svcname.toStdWString().c_str(), scf, this);

	if (0 == m_handle) {
		reportStatus(SERVICE_STOPPED);
		return;
	}

	if (reportStatus(SERVICE_START_PENDING))
		onStart(ac, av);

	return;
}

DWORD WINAPI
qservice_t::ServiceCtrl(DWORD ctrl, DWORD type, LPVOID edata, LPVOID ectxt)
{
	qservice_t* svc = static_cast< qservice_t* >(ectxt);


	if (nullptr == svc) {
		qservice_t::instance().getLog().error("Invalid context provided (null)", eventlog_cat_t::CAT_SERVICE, false);
		throw std::runtime_error("Invalid context provided (null)");
	}

	return svc->ctrlh(ctrl, type, edata);
}

DWORD 
qservice_t::ctrlh(DWORD ctrl, DWORD type, LPVOID edata)
{

	switch (ctrl) {
		case SERVICE_CONTROL_STOP:
			m_status.dwCurrentState = SERVICE_STOP_PENDING;
			reportStatus(SERVICE_STOP_PENDING);
			onStop();
			break;

		case SERVICE_CONTROL_PAUSE:
			m_status.dwCurrentState = SERVICE_PAUSE_PENDING;
			reportStatus(SERVICE_PAUSE_PENDING);
			onPause();
			break;

		case SERVICE_CONTROL_CONTINUE:
			m_status.dwCurrentState = SERVICE_CONTINUE_PENDING;
			reportStatus(SERVICE_CONTINUE_PENDING);
			onContinue();
			break;

		case SERVICE_CONTROL_SHUTDOWN:
			m_status.dwCurrentState = SERVICE_CONTROL_SHUTDOWN;
			reportStatus(SERVICE_STOP_PENDING);
			onShutdown();
			break;

		case SERVICE_CONTROL_INTERROGATE:
			reportStatus(m_status.dwCurrentState);
			break;

		default:
			return ERROR_CALL_NOT_IMPLEMENTED;
			break;
	}


	return NO_ERROR;

}

BOOL
qservice_t::reportStatus(DWORD state, DWORD hint, DWORD err)
{
	m_status.dwCurrentState		= state;
	m_status.dwWin32ExitCode	= NO_ERROR;
	m_status.dwWaitHint			= hint;

	if (state == SERVICE_START_PENDING) 
		m_status.dwControlsAccepted = 0;
	else 
		m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE|SERVICE_ACCEPT_SHUTDOWN;

	m_status.dwWin32ExitCode = err;

	if (0 != err)  // error exit code ... )
		m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;

	if (SERVICE_RUNNING == state || SERVICE_STOPPED  == state)
		m_status.dwCheckPoint = 0;
	else
		m_status.dwCheckPoint = m_checkpoint++;

	return ::SetServiceStatus(m_handle, &m_status);
}

LPSERVICE_MAIN_FUNCTION 
qservice_t::getServiceMainPointer(void) const
{
	return &qservice_t::ServiceMain;
}

void
qservice_t::run(qservice_t& s)
{
	LPSERVICE_MAIN_FUNCTION smf		= &qservice_t::ServiceMain;
	SERVICE_TABLE_ENTRY		dit[]	= {
		{ const_cast< LPTSTR >(s.m_svcname.toStdWString().c_str()), smf },
		{ nullptr, nullptr }
	};

	if (FALSE == ::StartServiceCtrlDispatcher(dit)) {
		qservice_t::instance().getLog().error("Error while calling StartServiceCtrlDispatcher()", eventlog_cat_t::CAT_SERVICE, false);
		throw std::runtime_error("Error while calling StartServiceCtrlDispatcher()");
	}

	return;
}

qservice_t&
qservice_t::instance(void)
{
	if (nullptr == qservice_t::m_instance) // XXX JF NO LOG (
		throw std::runtime_error("qservice_t::instance() called while m_instance is in invalid state (null)");
	

	return *qservice_t::m_instance;
}


typedef std::wstring::traits_type::char_type wcsize_t;


// XXX JF SECURITY REVIEW
LPWSTR 
qservice_t::intGetDependencies(void)
{
	LPWSTR				r(nullptr);
	std::size_t			l(0);
	QByteArray			b;

	for (auto itr = m_dependencies.begin(); itr != m_dependencies.end(); itr++) {
		std::wstring t = itr->toStdWString();

		b += QByteArray::fromRawData(reinterpret_cast< const char* >(t.c_str()), t.length()*sizeof(wcsize_t));
		b.append('\0');
		b.append('\0');
	}

	b.append('\0');
	b.append('\0');
	b.append('\0');
	b.append('\0');

	l = b.size();

	r = new WCHAR[l+1];
	::memcpy_s(r, l+1, b.data(), l);
	return r;
}

bool 
qservice_t::install(void)
{
	const std::size_t				blen(sizeof(wcsize_t));
	const std::size_t				slen(m_description.toStdWString().length() * blen);
	LPWSTR							deps = intGetDependencies();
	SC_HANDLE						mgr(nullptr);
	SC_HANDLE						svc(nullptr);
	SERVICE_DESCRIPTION				sde = {0};
	QString							bin(QCoreApplication::applicationFilePath());
	
	mgr = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT|SC_MANAGER_CREATE_SERVICE);

	if (nullptr == mgr) {
		m_log->error("OpenSCManager() failure");
		return false;
	}

	svc = ::CreateServiceW(mgr, m_svcname.toStdWString().c_str(), m_dispname.toStdWString().c_str(), 
							m_access, m_type, m_start, m_error,
							m_path.toStdWString().c_str(),
							m_loadgroup,
							nullptr, 
							deps, //nullptr,
							(0 == m_user.length() ? nullptr : static_cast< LPCWSTR >(m_user.toStdWString().c_str())),
							(0 == m_pass.length() ? nullptr : static_cast< LPCWSTR >(m_pass.toStdWString().c_str()))
						);


	if (nullptr == svc && ERROR_SERVICE_MARKED_FOR_DELETE == ::GetLastError()) {
		::Sleep(5000);

		svc = ::CreateServiceW(mgr, m_svcname.toStdWString().c_str(), m_dispname.toStdWString().c_str(), 
						m_access, m_type, m_start, m_error,
						m_path.toStdWString().c_str(),
						m_loadgroup,
						nullptr, 
						(0 == m_deps.length() ? nullptr : static_cast< LPCWSTR >(m_deps.toStdWString().c_str())), //nullptr,
						(0 == m_user.length() ? nullptr : static_cast< LPCWSTR >(m_user.toStdWString().c_str())),
						(0 == m_pass.length() ? nullptr : static_cast< LPCWSTR >(m_pass.toStdWString().c_str()))
					);	
	}

	if (nullptr == svc) {
		m_log->error("::CreateServiceW() failure");
		::CloseHandle(mgr);
		return false;
	}

	if (nullptr != deps)
		delete deps;

	sde.lpDescription = new WCHAR[slen+1];

	::memset(sde.lpDescription, 0, slen+1);
	::wmemcpy_s(sde.lpDescription, slen+1, m_description.toStdWString().c_str(), slen);

	if (FALSE == ::ChangeServiceConfig2W(svc, SERVICE_CONFIG_DESCRIPTION, &sde)) // non-fatal
		m_log->error("Failed to set service description via ::ChangeServiceConfig2(): ");
	

	delete sde.lpDescription;
	sde.lpDescription = nullptr;

	::CloseServiceHandle(svc);
	::CloseServiceHandle(mgr);
	return true;
}

bool 
qservice_t::remove(void)
{
	SC_HANDLE		mgr		= nullptr;
	SC_HANDLE		svc		= nullptr;
	SERVICE_STATUS	status	= {0};

	mgr = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);

	if (nullptr == mgr) {
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_STOP|SERVICE_QUERY_STATUS|DELETE);

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (false == this->stop()) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (! ::DeleteService(svc)) {
		m_log->error("::DeleteService() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

bool 
qservice_t::start(void)
{
	SC_HANDLE				mgr		= nullptr;
	SC_HANDLE				svc		= nullptr;
	LPCWSTR					av[2]	= { nullptr, nullptr };
	SERVICE_STATUS_PROCESS	stat	= {0};
	DWORD					len		= sizeof(stat);
	DWORD					stick	= 0;
	DWORD					ocheck	= 0;

	if (m_path.length()) 
		av[0] = m_path.toStdWString().c_str();
	else 
		av[0] = m_svcname.toStdWString().c_str();
	
	mgr = ::OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE|SC_MANAGER_LOCK);

	if (nullptr == mgr) {
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), STANDARD_RIGHTS_EXECUTE|SC_MANAGER_CONNECT|SC_MANAGER_LOCK|SERVICE_QUERY_STATUS|SERVICE_START);

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		m_log->error("::QueryServiceStatusEx() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED != stat.dwCurrentState && SERVICE_STOP_PENDING != stat.dwCurrentState) {
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
			m_log->error("::QueryServiceStatusEx() failure");
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			return false;
		}

		if (stat.dwCheckPoint > ocheck) {
			stick	= ::GetTickCount();
			ocheck	= stat.dwCheckPoint;
		} else {
			if (::GetTickCount() - stick > stat.dwWaitHint) {
				m_log->error("Timed out while waiting for service to start", eventlog_cat_t::CAT_SERVICE, false);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				return false;
			}
		}
	}

	if (FALSE == ::StartService(svc, 1, &av[0])) {
		m_log->error("::StartService() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		m_log->error("::QueryServiceStatusEx() failure");
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
			m_log->error("::QueryServiceStatusEx() failure");
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
		m_log->error("Timed out while waiting for service to start", eventlog_cat_t::CAT_SERVICE, false);
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	::CloseServiceHandle(mgr);
	::CloseServiceHandle(svc);
	return true;
}

bool 
qservice_t::stop(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		m_log->error("::QueryServiceStatusEx() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED == stat.dwCurrentState) {
		return true;
	}


	while (SERVICE_STOP_PENDING == stat.dwCurrentState) {

		if (1000 > wtime) 
			wtime = 1000;
		else if (10000 <= wtime)
			wtime = 10000;

		::Sleep(wtime);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			m_log->error("::QueryServiceStatusEx() failure");
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

	if (false == this->stopDependencies(mgr, svc)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}


	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&stat)) {
		m_log->error("::ControlService() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_STOPPED != stat.dwCurrentState)  
		::Sleep(1000);

	while (SERVICE_STOPPED != stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			m_log->error("::QueryServiceStatusEx() failure");
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
qservice_t::pause(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		m_log->error("::QueryServiceStatusEx() failure");
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
			m_log->error("::QueryServiceStatusEx() failure");
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

	if (FALSE == ::ControlService(svc, SERVICE_PAUSED, (LPSERVICE_STATUS)&stat)) {
		m_log->error("::ControlService() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_PAUSED != stat.dwCurrentState) 
		::Sleep(1000);

	while (SERVICE_PAUSED != stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			m_log->error("::QueryServiceStatusEx()");
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
qservice_t::resume(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
		m_log->error("::QueryServiceStatusEx() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_PAUSED != stat.dwCurrentState) 
		return false;

	if (FALSE == ::ControlService(svc, SERVICE_CONTROL_CONTINUE, (LPSERVICE_STATUS)&stat)) {
		m_log->error("::ControlService() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (SERVICE_RUNNING != stat.dwCurrentState) 
		::Sleep(1000);

	while (SERVICE_CONTINUE_PENDING == stat.dwCurrentState) {
		::Sleep(stat.dwWaitHint);

		if (FALSE == ::QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&stat, sizeof(SERVICE_STATUS_PROCESS), &len)) {
			m_log->error("::QueryServiceStatusEx() failure");
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
qservice_t::pauseDependencies(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (TRUE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, 0, &needed, &count)) {
		m_log->info("No dependencies to pause?");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);	
		return true;
	}

	if (ERROR_MORE_DATA != ::GetLastError()) {
		m_log->error("::EnumDependentServices() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		m_log->error("::HeapAlloc() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		m_log->error("::EnumDependentServices() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		::HeapFree(::GetProcessHeap(), 0, deps);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(mgr, ess.lpServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			m_log->error("::OpenService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_CONTINUE, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			m_log->error("::ControlService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_RUNNING != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				m_log->error("::QueryServiceStatusEx() failure");
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_RUNNING == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				m_log->error("Timed out while trying to stop dependent services", eventlog_cat_t::CAT_SERVICE, false);
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
qservice_t::resumeDependencies(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (TRUE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, 0, &needed, &count)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);	
		return true;
	}

	if (ERROR_MORE_DATA != ::GetLastError()) {
		m_log->error("::EnumDependentServices() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		m_log->error("::HeapAlloc() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		m_log->error("::EnumDependentServices() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		::HeapFree(::GetProcessHeap(), 0, deps);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(mgr, ess.lpServiceName, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			m_log->error("::OpenService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_PAUSE, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			m_log->error("::ControlService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_PAUSED != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				m_log->error("::QueryServiceStatusEx() failure");
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_PAUSED == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				m_log->error("Timed out while trying to stop dependent services", eventlog_cat_t::CAT_SERVICE, false);
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
qservice_t::stopDependencies(void)
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
		m_log->error("::OpenSCManager() failure");
		return false;
	}

	svc = ::OpenService(mgr, m_svcname.toStdWString().c_str(), SERVICE_ALL_ACCESS); 

	if (nullptr == svc) {
		m_log->error("::OpenService() failure");
		::CloseServiceHandle(mgr);
		return false;
	}

	if (TRUE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, 0, &needed, &count)) {
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return true;
	}

	if (ERROR_MORE_DATA != ::GetLastError()) {
		m_log->error("::EnumDependentServices() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		m_log->error("::HeapAlloc() failure");
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	if (FALSE == ::EnumDependentServices(svc, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		m_log->error("::EnumDependentServices() failure");
		::HeapFree(::GetProcessHeap(), 0, deps);
		::CloseServiceHandle(mgr);
		::CloseServiceHandle(svc);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(mgr, ess.lpServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			m_log->error("::OpenService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_STOP, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			m_log->error("::ControlService() failure");
			::CloseServiceHandle(deph);
			::CloseServiceHandle(mgr);
			::CloseServiceHandle(svc);
			::HeapFree(::GetProcessHeap(), 0, deps);
			return false;
		}

		while (SERVICE_STOPPED != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				m_log->error("::QueryServiceStatusEx() failure");
				::CloseServiceHandle(deph);
				::CloseServiceHandle(mgr);
				::CloseServiceHandle(svc);
				::HeapFree(::GetProcessHeap(), 0, deps);
				return false;
			}

			if (SERVICE_STOPPED == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				m_log->error("Timed out while trying to stop dependent services", eventlog_cat_t::CAT_SERVICE, false);
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
qservice_t::stopDependencies(SC_HANDLE m, SC_HANDLE h)
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
		m_log->error("Invalid parameters specified");
		return false;
	}

	if (TRUE == ::EnumDependentServices(h, SERVICE_ACTIVE, deps, 0, &needed, &count)) 
		return true;

	if (ERROR_MORE_DATA != ::GetLastError()) {
		m_log->error("::EnumDependentServices() failure");
		return false;
	}

	deps = static_cast< LPENUM_SERVICE_STATUS >(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, needed));

	if (nullptr == deps) {
		m_log->error("::HeapAlloc() failure");
		return false;
	}

	if (FALSE == ::EnumDependentServices(h, SERVICE_ACTIVE, deps, needed, &needed, &count)) {
		e = ::GetLastError();
		::HeapFree(::GetProcessHeap(), 0, deps);
		m_log->error("::EnumDependentServices() failure");
		::SetLastError(e);
		return false;
	}

	for (i = 0; i < count; i++) {
		ess = *(deps + i);

		deph = ::OpenService(m, ess.lpServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS);
				
		if (nullptr == deph) {
			e = ::GetLastError();
			::CloseServiceHandle(deph);
			::HeapFree(::GetProcessHeap(), 0, deps);
			m_log->error("::OpenService() failure");
			::SetLastError(e);
			return false;
		}

		if (FALSE == ::ControlService(deph, SERVICE_CONTROL_STOP, reinterpret_cast< LPSERVICE_STATUS >(&ssp))) {
			e = ::GetLastError();
			::CloseServiceHandle(deph);
			::HeapFree(::GetProcessHeap(), 0, deps);
			m_log->error("::ControlService() failure");
			::SetLastError(e);
			return false;
		}

		while (SERVICE_STOPPED != ssp.dwCurrentState) {
			::Sleep(ssp.dwWaitHint);

			if (FALSE == ::QueryServiceStatusEx(deph, SC_STATUS_PROCESS_INFO, reinterpret_cast< LPBYTE >(&ssp), sizeof(SERVICE_STATUS_PROCESS), &needed)) {
				e = ::GetLastError();
				::CloseServiceHandle(deph);
				::HeapFree(::GetProcessHeap(), 0, deps);
				m_log->error("::QueryServiceStatusEx() failure");
				::SetLastError(e);
				return false;
			}

			if (SERVICE_STOPPED == ssp.dwCurrentState)
				break;

			if (::GetTickCount() - start > tout) {
				e = ::GetLastError();
				::CloseServiceHandle(deph);
				::HeapFree(::GetProcessHeap(), 0, deps);
				m_log->error("Timed out while trying to stop dependent services", eventlog_cat_t::CAT_SERVICE, false);
				::SetLastError(e);
				return false;
			}
		}
	}

	return true;
}

void 
qservice_t::setAccessRights(DWORD a) 
{
	m_access = a;
	return;
}

DWORD 
qservice_t::getAccessRights(void)
{
	return m_access;
}

void 
qservice_t::setServiceType(DWORD t)
{
	m_type = t;
	return;
}

DWORD 
qservice_t::getServiceType(void)
{
	return m_type;
}

void 
qservice_t::setStartType(DWORD s)
{
	m_start = s;
	return;
}

DWORD 
qservice_t::getStartType(void)
{
	return m_start;
}

void 
qservice_t::setErrorControlType(DWORD e)
{
	m_error = e;
}

DWORD 
qservice_t::getErrorControlType(void)
{
	return m_error;
}

void 
qservice_t::setDependencies(QVector< QString >& d)
{
	m_dependencies = d;
	return;
}

void 
qservice_t::addDependencies(QString d)
{

	m_dependencies.push_back(d);
	return;
}

QString& 
qservice_t::getDependencies(void)
{
	return m_deps;
}

void 
qservice_t::setUser(QString u)
{
	m_user = u;
	return;
}

QString& 
qservice_t::getUser(void)
{
	return m_user;
}

void 
qservice_t::setPassword(QString p) 
{
	m_pass = p;
	return;
}

QString& 
qservice_t::getPassword(void)
{
	return m_pass;
}

eventLog_t& 
qservice_t::getLog(void)
{
	if (nullptr == m_log)
		m_log = new eventLog_t("Application", "QService");

	return *m_log;
}

void
qservice_t::setDescription(QString d)
{
	m_description = d;
	return;
}