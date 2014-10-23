#include "eventlogentry.h"

eventLogEntry_t::eventLogEntry_t(EVENTLOGRECORD& e) : QObject(nullptr)
{
	widec_t*	name(L"C:\\Program Files\\WINHTTPD\\winhttpd_messages.dll");



	m_dll = ::LoadLibraryEx(name, nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE);

	if (nullptr == m_dll) {
		util::error("Error in eventLogEntry_t::eventLogEntry_t()", "Error while loading eventlog resource DLL");
		throw std::runtime_error("eventLogEntry_t::eventLogEntry_t(): Error while loading eventlog resource DLL");
	}


	m_id			= e.RecordNumber;
	m_category		= getCategory(e.EventCategory);
	m_timestamp		= QDateTime::fromTime_t(e.TimeGenerated);
	m_type			= getType(e.EventType);

	if (1 == e.NumStrings) 
		m_message = getEventMessage(e);
	else // XXX JF FIXME
		throw std::runtime_error("eventLogEntry_t::eventLogEntry_t(): An unexpected amount of strings were encountered in the message body");

	m_message += getEventData(e);

	return;
}

eventLogEntry_t::~eventLogEntry_t(void)
{
	::FreeLibrary(m_dll);
	m_dll = nullptr;

	return;
}

QString 
eventLogEntry_t::getCategory(DWORD id)
{
	LPSTR	m = nullptr;
	QString r = "";

	if (FALSE == ::FormatMessageA(	FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ALLOCATE_BUFFER,
									m_dll,
									id,
									0,
									reinterpret_cast< LPSTR >(&m),
									0,
									nullptr)
								) {
									util::error("Error in eventLog_t::getCategory()", "Error while formating category string in log entry");
									throw std::runtime_error("eventLog_t::getCategory(): Error while formating category string in log entry");

	}

	r = m;
	::LocalFree(m);
	return r;
}

QString 
eventLogEntry_t::getType(WORD t)
{
	QString r("");

	switch (t) {
		case EVENTLOG_ERROR_TYPE:
			r = "Error";
			break;
		case EVENTLOG_WARNING_TYPE:
			r =  "Warning";
			break;
		case EVENTLOG_INFORMATION_TYPE:
			r =  "Informational";
			break;
		case EVENTLOG_AUDIT_SUCCESS:
			r =  "Audit Successful";
			break;
		case EVENTLOG_AUDIT_FAILURE:
			r =  "Audit Failure";
			break;
		default:
			throw std::runtime_error("evenLog_type::getLogEntry(): Received log entry for unknown type");
			break;
	}

	return r;
}

// XXX JF -
// FIXME entries with bogus offset's and similar
// ?? possible ??

QString 
eventLogEntry_t::getEventMessage(EVENTLOGRECORD& e)
{
	char*			p(nullptr);
	QString			r("");

	p = reinterpret_cast< char* >(&e);
	p += e.StringOffset;
	r = QString::fromWCharArray(reinterpret_cast< wchar_t* >(p));
	return r;
}

QString 
eventLogEntry_t::getEventData(EVENTLOGRECORD& e)
{
	char*			p(nullptr);
	QString			r("");

	if (0 < e.DataLength) {
		p = reinterpret_cast< char* >(&e);
		p += e.DataOffset; 
		r = " ";
		r += QString::fromWCharArray(reinterpret_cast< wchar_t* >(p));
	} 

	return r;
}
