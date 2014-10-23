#include "eventnotify.h"

Q_DECLARE_METATYPE(eventLogEntry_t::log_ptr_t)

eventNotify_t::eventNotify_t(QObject* p) : QObject(p), m_handle(nullptr), m_notify(nullptr), m_dll(nullptr), m_stop(false)
{
	DWORD status = ERROR_SUCCESS;

	qRegisterMetaType< eventLogEntry_t::log_ptr_t >();

	if (false == initHandles()) {
		emit errorMessage("Error in eventNotify_t::eventNotify_t()", "Error while initializing event log handles", true);
		throw std::runtime_error("eventNotify_t::eventNotify_t(): Error while initializing event log handles");
	}

	status = seekToLastRecord();

	if (ERROR_SUCCESS != status) {
		emit errorMessage("Error in eventNotify_t::eventNotify_t()", "Error while seeking to the end of the event log stream", true);
		throw std::runtime_error("eventNotify_t::eventNotify_t(): Error while seeking to the end of the event log stream");
	}
	
	return;
}

eventNotify_t::~eventNotify_t(void)
{
	if (nullptr != m_handle && INVALID_HANDLE_VALUE != m_handle) {
		::CloseEventLog(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}

	if (nullptr != m_dll) {
		::FreeLibrary(m_dll);
		m_dll = nullptr;
	}

	if (nullptr != m_notify && INVALID_HANDLE_VALUE != m_notify) {
		::CloseHandle(m_notify);
		m_notify = INVALID_HANDLE_VALUE;
	}

	return;
}

bool
eventNotify_t::initHandles(void)
{
	widec_t* name(L"C:\\Program Files\\WINHTTPD\\winhttpd_messages.dll");

	m_handle = ::OpenEventLog(nullptr, L"Application");

	if (nullptr == m_handle) {
		emit errorMessage("Error in evenLog_type::evenLog_type()", "Error while opening the event log", true);
		return false;
	}

	m_dll = ::LoadLibraryEx(name, nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE);

	if (nullptr == m_dll) {
		emit errorMessage("Error in evenLog_type::evenLog_type()", "Error while loading eventlog resource DLL", true);
		return false;
	}

	m_notify = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (nullptr == m_notify) {
		emit errorMessage("Error in evenLog_type::evenLog_type()", "Error while attempting to create event for event log notifications", true);
		return false;
	}

	return true;
}
void
eventNotify_t::start(void)
{
	DWORD retval(0);

	if (FALSE == ::NotifyChangeEventLog(m_handle, m_notify)) {
		emit errorMessage("Error in eventNotify_t::start()", "Error while registering to receive event notifications", true);
		//throw std::runtime_error("eventNotify_t::eventNotify_t(): Error while registering to receive event notifications");
		return;
	}

	while (false == m_stop) {
		retval = ::WaitForSingleObject(m_notify, INFINITE);

		if (true == m_stop)
			return;

		switch (retval) {
			case WAIT_OBJECT_0:
					::ResetEvent(m_notify);

					if (ERROR_SUCCESS != dumpNewRecords()) {
						emit errorMessage("Error in eventNotify_t::start()", "Error while processing new event log records", true);
						//throw std::runtime_error("eventNotify_t::start(): Error while processing new event log records");
						return;
					}
				break;

			case WAIT_TIMEOUT:
				break;

			case WAIT_ABANDONED: 
			case WAIT_FAILED:
			default:
				emit errorMessage("Error in evenLog_type::newStateThread()", "Error while polling the event notification event", true);
				//throw std::runtime_error("evenLog_type::newStateThread(): Error while polling the event notification event");
				return;
				break;
		}
	}

	return;
}

void
eventNotify_t::stop(void)
{
	m_stop = true;
	return;
}

DWORD 
eventNotify_t::getLastRecordNumber(DWORD& onum)
{
    DWORD old	= 0;
    DWORD num	= 0;

	if (FALSE == ::GetOldestEventLogRecord(m_handle, &old)) {
		emit errorMessage("Error in eventNotify_t::getLastRecordNumber()", "Error while retrieving the oldest event record number", true);
		return ::GetLastError(); 
	}

	if (FALSE == ::GetNumberOfEventLogRecords(m_handle, &num)) {
		emit errorMessage("Error in eventNotify_t::getLastRecordNumber()", "Error while retrieving the number of event log messages", true);
		return ::GetLastError(); 
	}

	if (0 == old + num)
		onum = 0;
	else 
		onum = old + num - 1;

	return ERROR_SUCCESS;
}

DWORD 
eventNotify_t::seekToLastRecord(void)
{
	DWORD sta = ERROR_SUCCESS;
	DWORD num = 0;
	PBYTE rec = nullptr;  

	sta = getLastRecordNumber(num);

	if (ERROR_SUCCESS != sta) {
		emit errorMessage("Error in eventNotify::seekToLastRecord()","Error while attempting to get the last record number", true);
		return sta;
	}

	sta = readRecord(rec, num, EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ);

	if (ERROR_SUCCESS != sta && ERROR_HANDLE_EOF != sta) {
		emit errorMessage("Error in eventNotify_t::seekToLastRecord()","Error while attempting to read event log record", true);
		return sta;
	}

	if (nullptr != rec)
		::free(rec);

	if (ERROR_HANDLE_EOF == sta)
		sta = ERROR_SUCCESS;

	return sta;
}

DWORD 
eventNotify_t::readRecord(PBYTE& pBuffer, DWORD num, DWORD f)
{
	DWORD sta = ERROR_SUCCESS;
	DWORD siz = sizeof(EVENTLOGRECORD);
	DWORD red = 0;
	DWORD min = 0;

	pBuffer = new BYTE[siz];

	if (FALSE == ::ReadEventLog(m_handle, f, num, pBuffer, siz, &red, &min)) {
		sta = ::GetLastError();

		if (ERROR_INSUFFICIENT_BUFFER == sta) {
			sta = ERROR_SUCCESS;

			delete pBuffer;
			pBuffer = new BYTE[min];
			siz = min;

			if (FALSE == ::ReadEventLog(m_handle, f, num, pBuffer, siz, &red, &min)) {
				emit errorMessage("Error in eventNotify_t::readRecord()", "Error while attempting to read the event log: ", true);
				return ::GetLastError();
			}
		} else {
			if (ERROR_HANDLE_EOF != sta) {
				emit errorMessage("Error in eventNotify_t::readRecord()", "Error while attempting to read the event log: ", true);
				return sta;
			} 
		}
	}

	return sta;
}

DWORD 
eventNotify_t::dumpNewRecords(void)
{
	EVENTLOGRECORD*				rec		= nullptr;
	BYTE*						ptr		= nullptr;
	eventLogEntry_t*			e		= nullptr;
	wchar_t*					wnam	= nullptr;
	QString						nam		= "";
    DWORD						status	= ERROR_SUCCESS;

	status = readRecord(ptr, 0, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ);
    
	if (ERROR_SUCCESS != status && ERROR_HANDLE_EOF != status) {
		if (nullptr != ptr)
			delete ptr;

		return status;
    }

	

    while (ERROR_HANDLE_EOF != status) {
		rec		= reinterpret_cast< EVENTLOGRECORD* >(ptr);
		wnam	= reinterpret_cast< wchar_t* >(reinterpret_cast< unsigned char* >(ptr + sizeof(EVENTLOGRECORD)));

		if (nullptr == wnam) {
			delete ptr;
			ptr = nullptr;
			return ERROR_INVALID_PARAMETER; // XXX JF FIXME
		}

		nam = QString::fromWCharArray(wnam);

		if (! nam.compare("WINHTTPD-Controller", Qt::CaseInsensitive) || ! nam.compare("WINHTTPD-Worker", Qt::CaseInsensitive)) {
			e = new eventLogEntry_t(*rec); //std::make_shared< eventLogEntry_t >(*rec);
			emit addRecord(e);
			//e = nullptr;
		}

		delete ptr;
		ptr = nullptr;

		status = readRecord(ptr, 0, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ);

        if (ERROR_SUCCESS != status && ERROR_HANDLE_EOF != status) {
            if (nullptr != ptr)
				delete ptr;

			return status;
        }
    }

    if (ERROR_HANDLE_EOF == status) 
        status = ERROR_SUCCESS;

    return status;
}