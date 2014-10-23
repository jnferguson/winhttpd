#include "eventlog.h"

Q_DECLARE_METATYPE(eventLogEntry_t::log_ptr_t)

eventLog_t::eventLog_t(QObject* p) : QObject(p), m_enotify(nullptr), m_firstStart(false)
{
	std::lock_guard< std::mutex >	l(m_mutex);

	qRegisterMetaType< eventLogEntry_t::log_ptr_t >();

	if (false == initializeHandles())
		throw std::runtime_error("evenLog_type::evenLog_type(): Error while initializing event log associated handles");

	m_queueTimer	= new QTimer(this);

	connect(m_queueTimer, &QTimer::timeout, this, &eventLog_t::queueThread);

	m_queueTimer->setInterval(500);


	return;
}

eventLog_t::~eventLog_t(void)
{
	std::lock_guard< std::mutex >	l(m_mutex);

	m_queueTimer->stop();

	destroyHandles();
	destroyQueue();
	return;
}

void
eventLog_t::start(void)
{
	std::lock_guard< std::mutex >	l(m_mutex);
	QThread*						t = new QThread;

	if (false == m_firstStart) {
		getAllEntries();
		m_queueTimer->start();
		m_firstStart = true;
	}

	if (nullptr != m_enotify)
		delete m_enotify;

	m_enotify = new eventNotify_t(nullptr);
	m_enotify->moveToThread(t);

	connect(t, &QThread::started, m_enotify, &eventNotify_t::start);
	connect(t, &QThread::finished, t, &QThread::deleteLater);
	connect(m_enotify, &eventNotify_t::addRecord, this, &eventLog_t::recvRecord);
	connect(m_enotify, &eventNotify_t::errorMessage, this, &eventLog_t::recvErrorMessage);

	t->start();
	return;
}

HANDLE 
eventLog_t::getEventLogHandle(void) 
{
	std::lock_guard< std::mutex > l(m_handleMutex);
	return m_handle;
}

HMODULE
eventLog_t::getDllHandle(void)
{
	std::lock_guard< std::mutex > l(m_handleMutex);
	return m_dll;
}

HANDLE
eventLog_t::getNotifyHandle(void)
{
	std::lock_guard< std::mutex > l(m_handleMutex);
	return m_notify;
}

bool 
eventLog_t::initializeHandles(void)
{
	std::lock_guard< std::mutex >	l(m_handleMutex);
	widec_t*						name(L"C:\\Program Files\\WINHTTPD\\winhttpd_messages.dll");

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

	return true;
}

void
eventLog_t::destroyHandles(void)
{
	std::lock_guard< std::mutex > l(m_handleMutex);

	if (nullptr != m_handle && INVALID_HANDLE_VALUE != m_handle) {
		::CloseEventLog(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}

	if (nullptr != m_dll) {
		::FreeLibrary(m_dll);
		m_dll = nullptr;
	}

	return;
}

void 
eventLog_t::addRecord(eventLogEntry_t* e)
{
	std::lock_guard< std::mutex > l(m_queueMutex);

	if (nullptr == e)
		return;

	m_queue.append(e);
	return;
}

eventLogEntry_t* 
eventLog_t::getRecord(void)
{
	std::lock_guard< std::mutex >	l(m_queueMutex);

	if (m_queue.isEmpty())
		return nullptr;

	return m_queue.takeLast();
}

bool 
eventLog_t::isQueueEmpty(void)
{
	std::lock_guard< std::mutex > l(m_queueMutex);

	return m_queue.isEmpty();
}

void 
eventLog_t::destroyQueue(void)
{
	std::lock_guard< std::mutex > l(m_queueMutex);

	m_queue.clear();
	return;
}

void 
eventLog_t::queueThread(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	while (false == isQueueEmpty()) {
		eventLogEntry_t* e = getRecord();

		if (nullptr == e)
			continue;

		emit entryReady(e);
	}

	return;
}

void 
eventLog_t::getEntries(quint64 oldest, quint64 newest)
{
	for (quint64 current = oldest; current < newest; current++)
		getLogEntry(current);

	return;
}

quint64
eventLog_t::getNewestRecordId(void)
{
	std::lock_guard< std::mutex >	l(m_recordIdMutex);
	DWORD							o(0);
	DWORD							r(0);
	DWORD							c(0);

	if (FALSE == ::GetOldestEventLogRecord(getEventLogHandle(), &o)) {
		emit errorMessage("Error in eventlog_t::getNewestRecordId()", "Error while attempting to get oldest event log record id", true);
		throw std::runtime_error("evenLog_t::getNewestRecordId(): Error while attempting to get oldest event log record id");
		return static_cast< quint64 >(0);
	}

	if (FALSE == ::GetNumberOfEventLogRecords(getEventLogHandle(), &c)) {
			emit errorMessage("Error in evenLog_t::getNewestRecordId()", "Error while attempting to get event log record count", true);
			throw std::runtime_error("evenLog_t::getNewestRecordId(): Error while attempting to get event log record count");
			return static_cast< quint64 >(0);	
	}

	if (0 == o+c) // no log entries
		return static_cast< quint64 >(0); 

	r = o + c - 1;
	return static_cast< quint64 >(r);
}

quint64 
eventLog_t::getOldestRecordId(void)
{
	std::lock_guard< std::mutex >	l(m_recordIdMutex);
	DWORD							r = 0;

	if (FALSE == ::GetOldestEventLogRecord(getEventLogHandle(), &r)) {
		emit errorMessage("Error in evenLog_t::getOldestRecordId()", "Error while attempting to get oldest event log record id", true);
		throw std::runtime_error("evenLog_t::getOldestRecordId(): Error while attempting to get oldest event log record id");
	}

	return r;
}

void
eventLog_t::getAllEntries(void)
{
	DWORD							oldest = getOldestRecordId();
	DWORD							newest = getNewestRecordId();

	for (DWORD current = oldest; current < newest; current++) 
		getLogEntry(current);

	return;
}

void 
eventLog_t::getLogEntry(quint64 id)
{
	eventLogEntry_t*				e(nullptr);
	wchar_t*						wnam(nullptr);
	QString							nam("");
	EVENTLOGRECORD*					rec(nullptr);
	BYTE*							ptr(nullptr);
	DWORD							siz(0);
	DWORD							min(0);
	DWORD							red(0);

	siz = 1;
	ptr = new BYTE[siz];

	// XXX JF FIXME - checks to ensure ID is not > DWORD max
	// quint64 used for precisely this reason as initialized value for m_oldest (0)
	// in theory could be a valid record id, so the initialized value will eventually
	// be > DWORD max value
	// ... eventually.

	if (FALSE == ::ReadEventLog(getEventLogHandle(), EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ, static_cast< DWORD >(id), ptr, siz, &red, &min)) {
		DWORD stat = ::GetLastError();

		if (ERROR_INSUFFICIENT_BUFFER  == stat) {
			delete ptr;
			ptr = new BYTE[min];
			siz = min;

			if (FALSE == ::ReadEventLog(getEventLogHandle(), EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ, static_cast< DWORD >(id), ptr, siz, &red, &min)) {
				if (ERROR_HANDLE_EOF != ::GetLastError()) {
					emit errorMessage("Error in evenLog_type::getLogEntry()","Error while attempting to read event log: ", true);
					return;
				}
			}

		} else {
			if (ERROR_HANDLE_EOF != ::GetLastError()) {
				emit errorMessage("Error in evenLog_type::getLogEntry()","Error while attempting to read event log: ", true);
				return;
			}
		}
	}

	rec		= reinterpret_cast< EVENTLOGRECORD* >(ptr);
	wnam	= reinterpret_cast< wchar_t* >(reinterpret_cast< unsigned char* >(ptr + sizeof(EVENTLOGRECORD)));

	if (nullptr == wnam) {
		delete ptr;
		ptr = nullptr;
		return;
	}

	nam = QString::fromWCharArray(wnam);
	
	if (! nam.compare("WINHTTPD-Controller", Qt::CaseInsensitive) || ! nam.compare("WINHTTPD-Worker", Qt::CaseInsensitive)) {
		e = new eventLogEntry_t(*rec); 
		addRecord(e);
		e = nullptr;
	}

	delete ptr;
	ptr = nullptr;

	return;
}

void
eventLog_t::recvRecord(eventLogEntry_t* e)
{
	addRecord(e);
	return;
}

void
eventLog_t::recvErrorMessage(const QString& t, const QString& m, bool gle)
{
	emit errorMessage(t,m,gle);
	return;
}

void
eventLog_t::clearEventLog(void)
{
	if (FALSE == ::ClearEventLog(getEventLogHandle(), nullptr)) 
		emit errorMessage("Error in evenLog_type::clearEventLog()","Error while attempting to clear the event log: ", true);
	
	m_enotify->stop();
	QThread::msleep(250);
	start();

	return;
}