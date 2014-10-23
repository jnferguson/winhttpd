#include "tlog_handler.h"


namespace http {

	tlog_handler_t::tlog_handler_t(workerLog_t& l) : handler_t(l), m_time(nullptr)
	{
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
		QString		d("");


		s.beginGroup("Plugins/FlatTextFileLogger");

		if (!s.value("Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_log.error("tlog_handler_t::tlog_handler_t(): Plugin initialized when disabled in configuration", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("tlog_handler_t::tlog_handler_t(): Plugin initialized when disabled in configuration");
			return;
		}

		m_name		= s.value("Name", "FlatTextFileLogger").toString();
		m_filename	= s.value("Options/ConnectionLogFile", "ConnectionLog").toString();
		m_rotate	= s.value("Options/RotateLogFile", true).toBool();
		d			= s.value("Options/ConnectionLogPath", QCoreApplication::applicationDirPath() + QDir::separator() + "logs").toString();

		m_dir.setPath(d);

		if (!m_dir.exists()) {
			m_log.error("tlog_handler_t::tlog_handler_t(): Logging directory does not exist", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("tlog_handler_t::tlog_handler_t(): Logging directory does not exist");
			return;
		}

		if (false == openLogFile()) {
			m_log.error("tlog_handler_t::tlog_handler_t(): Error while opening log file", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("tlog_handler_t::tlog_handler_t(): Error while opening log file");
			return;
		}

		s.endGroup();

		m_log.info("tlog_handler: plugin initialized");

		return;
	}

	tlog_handler_t::~tlog_handler_t(void)
	{
		closeLogFile();
		return;
	}

	HTTP_HANDLER_RETURN_T
	tlog_handler_t::operator()(http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		http::resp_sptr_t	respPtr = resp.lock();
		http::req_sptr_t	reqPtr = req.lock();

		if (respPtr && reqPtr)
			return operator()(reqPtr, respPtr);


		return HTTP_HANDLER_RETURN_DECLINED;
	}

	HTTP_HANDLER_RETURN_T
	tlog_handler_t::operator()(http::req_sptr_t& req, http::resp_sptr_t& resp)
	{
		std::lock_guard< std::mutex >	l(m_mutex);
		QString							entry("");
		QDateTime						now(QDateTime::currentDateTime());

		if (nullptr == req.get() || nullptr == resp.get()) {
			m_log.error("tlog_handler_t::operator(): Invalid parameter passed to logger", eventlog_cat_t::CAT_WORKER, false);
			return HTTP_HANDLER_RETURN_DECLINED;
		}

		entry += "[" + now.toString("dd-MMM-yyyy hh:mm:ss.zzzz]: ");
		entry += req->getRemoteAddress() + " "; // username or "-" ;
		entry += req->getVerbString() + " " + req->getAbsolutePath() + " ";
		entry += QString::number(resp->getStatus()) + "\r\n";

		// XXX JF FIXME
		if (entry.length() > m_file.write(entry.toUtf8())) {
			m_log.error("tlog_handler_t::operator(): Error writing to log file, entry either truncated or omitted");
			return HTTP_HANDLER_RETURN_DECLINED;
		}

		m_file.flush();
		return HTTP_HANDLER_RETURN_OKAY;
	}

	bool
	tlog_handler_t::openLogFile(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);
		QDateTime						ts(QDateTime::currentDateTime());
		QString							fn(QDir::toNativeSeparators(m_dir.absolutePath() + "\\" + m_filename));
		QIODevice::OpenMode				mode(QIODevice::WriteOnly | QIODevice::Text);

		fn += "-" + ts.toString("dd-MMM-yyyy");
		fn += ".log";

		m_file.setFileName(fn);

		if (m_file.exists())
			mode |= QIODevice::Append;

		if (false == m_file.open(mode)) {
			m_log.error("tlog_handler_t::openLogFile(): Error opening log file.");
			return false;
		}

		if (true == m_rotate) {
			if (nullptr != m_time) {
				m_time->stop();
				delete m_time;
			}

			m_time = new rotate_timer_t(std::function< void(void) >(std::bind(&tlog_handler_t::rotateLogFile, this)));
		}

		return true;
	}

	bool
	tlog_handler_t::closeLogFile(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		if (m_file.isOpen())
			m_file.close();

		return true;
	}

	void
	tlog_handler_t::rotateLogFile(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		if (false == closeLogFile()) {
			m_log.error("tlog_handler_t::rotateLogFile(): Error while attempting to close log file");
			return;
		}

		if (false == openLogFile()) {
			m_log.error("tlog_handler_t::rotateLogFile(): Error while attempting to open new log file");
			return;
		}

		return;
	}

	rotate_timer_t::rotate_timer_t(std::function< void(void) >& cb) : m_callback(cb)
	{
		this->reset();
		return;
	}

	rotate_timer_t::~rotate_timer_t(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		return;
	}

	void
	rotate_timer_t::reset(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);
		QDateTime						ts(QDateTime::currentDateTime());
		qint64							tt(0);

		m_timer.stop();
		ts.addDays(1);
		tt = ts.msecsTo(QDateTime::fromString(ts.toString("dd-MMM-yyyy") + " 00:00:00", "dd-MMM-yyyy hh:mm:ss"));

		connect(&m_timer, &QTimer::timeout, this, &rotate_timer_t::execute);
		m_timer.start(tt);

		return;
	}

	void
	rotate_timer_t::stop(void)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		m_timer.stop();
		return;
	}

	void
	rotate_timer_t::start(void)
	{
		this->reset();
		return;
	}

	void
	rotate_timer_t::execute(void)
	{
		m_callback();
		return;
	}
}