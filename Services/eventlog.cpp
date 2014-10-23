#include "eventlog.h"


eventLog_t::eventLog_t(QString n, QString s, QObject *p)
	: QObject(p),
	m_name(n),
	m_source(s)
{
	std::lock_guard< std::mutex > l(m_mutex);

	//this->createEventSource();

	m_handle = ::RegisterEventSource(nullptr, m_source.toStdWString().c_str());

	if (nullptr == m_handle)
		throw std::runtime_error("Error while registering event source");

	return;

}

eventLog_t::~eventLog_t()
{
	std::lock_guard< std::mutex > l(m_mutex);

	::DeregisterEventSource(m_handle);
	m_handle = nullptr;
	return;
}

void 
eventLog_t::writeEvent(const QString& m, const WORD t, eventlog_cat_t c)
{
	std::lock_guard< std::mutex >	l(m_mutex);
	DWORD							eid = 0;
	WCHAR*							ptr = new WCHAR[m.length()+1];
	LPCWSTR							msg = nullptr; 

	::memset(ptr, 0, m.length()+1*sizeof(WCHAR));
	m.toWCharArray(ptr);
	ptr[m.length()] = '\0';
	msg = ptr;
	
	switch (t) {
		case EVENTLOG_SUCCESS:
			eid = MSG_EL_SUCCESS;
			break;
		case EVENTLOG_ERROR_TYPE:
			eid = MSG_EL_ERROR;
			break;
		case EVENTLOG_WARNING_TYPE:
			eid = MSG_EL_WARNING;
			break;
		case EVENTLOG_INFORMATION_TYPE:
			eid = MSG_EL_INFO;
			break;
		case EVENTLOG_AUDIT_SUCCESS:
			eid = MSG_EL_AUDIT_SUCCESS;
			break;
		case EVENTLOG_AUDIT_FAILURE:
			eid = MSG_EL_AUDIT_FAILURE;
			break;
		default:
			throw std::runtime_error("Unknown/invalid event log type specified");
			break;
	}

	if (FALSE == ::ReportEventW(m_handle, t, c, eid, nullptr, 1, 0, &LPCWSTR(msg), nullptr)) 
		throw std::runtime_error("Failed to write to the event log");

	return;
}

void 
eventLog_t::info(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_INFORMATION_TYPE, c);
	return;
}

void 
eventLog_t::warning(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_WARNING_TYPE, c);
	return;
}

void 
eventLog_t::error(QString m, eventlog_cat_t c, bool gle)
{
	if (true == gle) 
		m += " " + this->loadErrorMessage();

	writeEvent(m, EVENTLOG_ERROR_TYPE, c);
	return;
}

void 
eventLog_t::success(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_SUCCESS, c);
	return;
}

void 
eventLog_t::auditSuccess(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_SUCCESS, c);
	return;
}

void 
eventLog_t::auditFailure(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_FAILURE, c);
	return;
}

void
eventLog_t::clear(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	::ClearEventLog(m_handle, nullptr);
	return;
}

DWORD
eventLog_t::numberOfEventLogEntries(void)
{
	std::lock_guard< std::mutex >	l(m_mutex);
	DWORD							n(0);

	if (FALSE == ::GetNumberOfEventLogRecords(m_handle, &n)) 
		throw std::runtime_error("Error while retrieving event log entry count");

	return n;
}

typedef std::wstring::traits_type::char_type widec_t;

void
eventLog_t::createEventSource(void)
{
	std::size_t		blen(sizeof(widec_t));
	widec_t*		name(L"C:\\Program Files\\WINHTTPD\\winhttpd_messages.dll");
	std::size_t		nlen(::wcslen(name)*sizeof(blen));
	QString			path("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\" + m_name);
	HKEY			key(nullptr);
	DWORD			lerr(NO_ERROR);
	const DWORD		ccnt = SERVICE_CATEGORY;
	DWORD			supp = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE
							| EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS | EVENTLOG_AUDIT_FAILURE;
	
	lerr = ::RegCreateKeyExW(HKEY_LOCAL_MACHINE, path.toStdWString().c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr);

	if (ERROR_SUCCESS != lerr) 
		throw std::runtime_error("eventlog_t::createEventSource() error while attempting to create registry key");

	lerr = ::RegSetValueExW(key, L"EventMessageFile", 0, REG_SZ, reinterpret_cast< const BYTE* >(name), nlen);

	if (ERROR_SUCCESS != lerr)
		throw std::runtime_error("eventlog_t::createEventSource() error while attempting to write registry key");

	lerr = ::RegSetValueExW(key, L"CategoryMessageFile", 0, REG_SZ, reinterpret_cast< const BYTE* >(name), nlen);

	if (ERROR_SUCCESS != lerr)
		throw std::runtime_error("eventlog_t::createEventSource() error while attempting to write registry key");

	lerr = ::RegSetValueExW(key, L"TypesSupported", 0, REG_DWORD, reinterpret_cast< BYTE* >(&supp), sizeof(supp));

	if (ERROR_SUCCESS != lerr)
		throw std::runtime_error("eventlog_t::createEventSource() error while attempting to write registry key");

	lerr = ::RegSetValueExW(key, L"CategoryCount", 0, REG_DWORD, reinterpret_cast< const BYTE* >(&ccnt), sizeof(ccnt));

	if (ERROR_SUCCESS != lerr)
		throw std::runtime_error("eventlog_t::createEventSource() error while attempting to write registry key");

	::RegCloseKey(key);

	return;
}

void
eventLog_t::deleteEventSource(void)
{
	QString		path("HKEY_CURRENT_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application" + m_name);
	QSettings	regs(path, QSettings::NativeFormat);

	regs.remove(m_name);
	return;
}

QString 
eventLog_t::loadMessage(DWORD id, ...)
{
	LPWSTR	ptr = nullptr;
	va_list arg	= nullptr;
	DWORD	ret = 0;
	QString str = "";

	va_start(arg, id);

	ret = ::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE,
						nullptr,
						id,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						reinterpret_cast< LPWSTR >(&ptr),
						0,
						&arg
				);

	if (0 == ret) 
		throw std::runtime_error("Failed to format log message");

	if (nullptr == ptr)
		throw std::runtime_error("::FormatMessage() returned successfully, but yielded a nullptr");

	str = QString::fromWCharArray(ptr);
	::LocalFree(ptr);
	return str;
}

QString 
eventLog_t::loadErrorMessage(DWORD e)
{
	LPWSTR	ptr = nullptr;
	DWORD	ret = 0;
	QString str = "";

	ret = ::FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					e,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					reinterpret_cast< LPWSTR >(&ptr),
					0,
					nullptr
			);

	if (0 == ret) 
		throw std::runtime_error("Failed to format log message");

	if (nullptr == ptr)
		throw std::runtime_error("::FormatMessage() returned successfully, but yielded a nullptr");

	str = QString::fromWCharArray(ptr);
	::LocalFree(ptr);
	return str;
} 