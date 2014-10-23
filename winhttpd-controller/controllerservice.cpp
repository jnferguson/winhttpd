#include "controllerservice.h"


controllerService_t::controllerService_t(QString n, QString d, QString e, QString l, QObject* p) : qservice_t(n,d,e,l,p), m_http(nullptr), m_thread(nullptr), m_state(QSERVICE_INVALID)
{
	m_log		= std::unique_ptr< controllerLog_t >(new controllerLog_t(nullptr));
	m_http		= std::unique_ptr< http_controller_t >(new http_controller_t(*m_log));
	m_thread	= std::unique_ptr< std::thread >(new std::thread(std::bind(&controllerService_t::fsm, this)));
	
	return;
}

controllerService_t::~controllerService_t()
{
	std::lock_guard< std::mutex > l(m_mutex);

	m_state = QSERVICE_STOPPED;
	QThread::msleep(1000);

	m_http->stop();
	
	m_http		= nullptr;
	m_thread	= nullptr;
	m_log		= nullptr;
	
	return;
}

void 
controllerService_t::setState(qservice_state_t s)
{
	std::lock_guard< std::mutex > l(m_mutex);

	m_state = s;
	return;
}

qservice_t::qservice_state_t 
controllerService_t::getState(void)
{
	std::lock_guard< std::mutex > l(m_mutex);
	return m_state;
}

void
controllerService_t::fsm(void)
{
	while (1) {
		switch (this->getState()) {
			case QSERVICE_STARTING:
			
				setState(QSERVICE_RUNNING);
				break;

			case QSERVICE_RUNNING:
				
				processRequests();
				break;

			case QSERVICE_STOPPED:
				
				if (false == this->stopDependencies()) 
					m_log->error("Error while attempting to stop dependent services", eventlog_cat_t::CAT_CONTROLLER, false);
				

				QCoreApplication::quit();
				break;

			case QSERVICE_PAUSED:
				
				if (false == this->pauseDependencies()) 
					m_log->error("Error while attempting to pause dependent services, execution state is undefined upon resumption of the controller service", eventlog_cat_t::CAT_CONTROLLER, false);

				QThread::msleep(500);
				break;

			case QSERVICE_RESUME:
				
				if (false == this->resumeDependencies())
					m_log->error("Error while attempting to resume dependent services, execution state is undefined upon resumption of the controller service", eventlog_cat_t::CAT_CONTROLLER, false);

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
controllerService_t::processRequests(void)
{

	if (false == m_http->start()) {
		m_log->error("Error while starting HTTP server", eventlog_cat_t::CAT_CONTROLLER, false);
		onStop();
		return;
	}

	while (QSERVICE_RUNNING == getState()) {
		if (false == m_http->waitForRequest()) {
			m_log->error("Error while waiting for HTTP client", eventlog_cat_t::CAT_CONTROLLER, false);
			onStop();
			return;
		}
	}

	m_http->stop();

	return;
}

void 
controllerService_t::onStart(DWORD, LPTSTR*)
{
	m_log->info("Service started");

	this->reportStatus(SERVICE_RUNNING);
	setState(QSERVICE_STARTING);
	emit started();
	return;
}

void 
controllerService_t::onStop(void)
{

	m_log->info("Received stop request, shutting down service.");
	this->reportStatus(SERVICE_STOP_PENDING);
	setState(QSERVICE_STOPPED);
	this->reportStatus(SERVICE_STOPPED);
	emit stopped();
	return;
}

void 
controllerService_t::onPause(void)
{
	m_log->info("Received pause request, pausing service.");
	setState(QSERVICE_PAUSED);
	reportStatus(SERVICE_PAUSED);
	emit paused();
	return;
}

void 
controllerService_t::onContinue(void)
{
	m_log->info("Received resume request, resuming service service.");
	reportStatus(SERVICE_START_PENDING);
	setState(QSERVICE_STARTING);
	reportStatus(SERVICE_RUNNING);
	emit resumed();
	return;
}

void 
controllerService_t::onShutdown(void)
{
	m_log->info("Received shutdown request, shutting down service.");
	setState(QSERVICE_STOPPED);
	this->reportStatus(SERVICE_STOPPED);
	emit shutdown();
	return;
}