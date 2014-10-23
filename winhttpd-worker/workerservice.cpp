#include "workerservice.h"

workerService_t::workerService_t(QString n, QString d, QString e, QString l, QObject* p) : qservice_t(n, d, e, l, p), m_http(nullptr), m_log(nullptr), m_wthr(nullptr), m_state(QSERVICE_INVALID), m_handlers(nullptr)
{
	std::lock_guard< std::mutex > lck(m_mutex);

	m_log		= std::unique_ptr< workerLog_t >(new workerLog_t(nullptr));
	m_http		= std::unique_ptr< http::worker_t >(new http::worker_t(*m_log));
	m_handlers	= std::unique_ptr< http::handler_mgr_t >(new http::handler_mgr_t(*m_log));
	m_wthr		= std::unique_ptr< std::thread >(new std::thread(std::bind(&workerService_t::fsm, this)));
	return;
}

workerService_t::~workerService_t(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	m_state = QSERVICE_STOPPED;
	QThread::msleep(1000);
	
	m_http->stop();

	m_http	= nullptr;
	m_wthr	= nullptr;
	m_log	= nullptr;

	return;
}

void 
workerService_t::setState(qservice_state_t s)
{
	std::lock_guard< std::mutex > l(m_mutex);

	m_state = s;
	return;
}

qservice_t::qservice_state_t 
workerService_t::getState(void)
{
	std::lock_guard< std::mutex > l(m_mutex);
	return m_state;
}

void
workerService_t::fsm(void)
{
	while (1) {
		switch (this->getState()) {
			case QSERVICE_STARTING:

				setState(QSERVICE_RUNNING);
				processRequests();
				break;

			case QSERVICE_RUNNING:
				
				QThread::msleep(1000);
				break;

			case QSERVICE_STOPPED:
							
				m_http->stop();
				m_http	= nullptr;
				m_wthr	= nullptr;
				m_log	= nullptr;
				QCoreApplication::quit();
				break;

			case QSERVICE_PAUSED:

				m_http->stop();
				QThread::msleep(500);
				break;

			case QSERVICE_RESUME:

				setState(QSERVICE_STARTING);
				break;

			case QSERVICE_INVALID:

				QThread::msleep(500);
				break;

			default:

				m_log->info("Invalid state encountered");
				setState(QSERVICE_STOPPED);
				break;
		}
	}

	return;
}

void
workerService_t::processRequests(void)
{
	
	if (false == m_http->start()) {
		m_log->error("workerService_t::processRequests(): Error while calling http::worker_t::start()", eventlog_cat_t::CAT_WORKER, false);
		m_http->stop();
		onStop();
		return;
	}

	while (QSERVICE_RUNNING == getState()) {
		try {
			if (false == m_http->inputQueueIsEmpty()) { 
				http::req_sptr_t			req(m_http->getRequestFromInputQueue());
				http::resp_sptr_t			resp(http::resp_sptr_t(new http::resp_t(*m_log)));

				if (nullptr == req) {
					m_log->error("workerService_t::processRequests(): Error while calling low_level_http_t::getRequestFromInputQueue()", eventlog_cat_t::CAT_WORKER, false);
					m_http->stop();
					onStop();
					return;	
				}

				resp->setId(req->getId());

				if (false == m_handlers->executeHandlers(http::req_wptr_t(req), http::resp_wptr_t(resp))) {
					m_log->error("workerService_t::processRequests(): Error while calling http::handler_mgr_t::executeHandlers()", eventlog_cat_t::CAT_WORKER, false);
					req = nullptr;
					resp = nullptr;
					continue;
				}

				resp->addHeader(QString("Server"), QString("WINHTTPD v0.1"));
				resp->addHeader(QString("X-Unknown-Header-Type"), QString("UnknownHeaderTypeValue"));

				m_http->putResponseOnOutputQueue(resp);
				resp	= nullptr; 
				req		= nullptr;

			} else 
				QThread::msleep(250);

		} catch (std::runtime_error& e) {
				QString err("workerService_t::processRequests() Error: ");

				err += e.what();
				m_log->error(err, eventlog_cat_t::CAT_WORKER, false);

		} catch (...) {
			m_log->error("workerService_t::processRequests(): unknown exception caught");
		}
	}

	m_http->stop();
	return;
}

void 
workerService_t::onStart(DWORD, LPTSTR*)
{
	m_log->info("Service started");
	setState(QSERVICE_STARTING);
	reportStatus(SERVICE_RUNNING);
	emit started();
	return;
}

void 
workerService_t::onStop(void)
{
	m_log->info("Received stop request, shutting down service.");
	reportStatus(SERVICE_STOP_PENDING);
	emit stopped();
	reportStatus(SERVICE_STOPPED);
	setState(QSERVICE_STOPPED);
	return;
}

void 
workerService_t::onPause(void)
{
	m_log->info("Received pause request, pausing service.");
	setState(QSERVICE_PAUSED);
	reportStatus(SERVICE_PAUSED);
	emit paused();
	return;
}

void 
workerService_t::onContinue(void)
{
	m_log->info("Received resume request, resuming service service.");
	setState(QSERVICE_RESUME);
	reportStatus(SERVICE_RUNNING);
	emit resumed();
	return;
}

void 
workerService_t::onShutdown(void)
{
	m_log->info("Received shutdown request, shutting down service.");
	this->reportStatus(SERVICE_STOPPED);
	emit shutdown();
	setState(QSERVICE_STOPPED);
	return;
}

workerLog_t&
workerService_t::getLogger(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	if (nullptr == m_log) 
		throw std::runtime_error("Log pointer is null");
	
	return *m_log;
}
