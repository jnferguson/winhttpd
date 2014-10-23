#ifndef WORKERSERVICE_H
#define WORKERSERVICE_H

#include <QObject>
#include <QThread>
#include <thread>
#include <memory>

#include "qservice.h"
#include "workerlog.h"
#include "http_worker.h"
#include "http_resp.h"
#include "http_req.h"
#include "handler.h"

class workerService_t : public qservice_t
{
	Q_OBJECT

	private:
		qservice_state_t						m_state;
		std::unique_ptr< std::thread >			m_wthr;
		std::unique_ptr< workerLog_t >			m_log;
		std::unique_ptr< http::worker_t >		m_http;
		std::unique_ptr< http::handler_mgr_t >	m_handlers;
		std::mutex								m_mutex;
		//threadpool_t*		m_tpool;

	protected:
		virtual void setState(qservice_state_t);
		virtual qservice_t::qservice_state_t getState(void);
		virtual void fsm(void);

		void processRequests(void);

	public:
		workerService_t(QString, QString, QString, QString, QObject* p = nullptr);
		~workerService_t(void);

		virtual void onStart(DWORD, LPTSTR*);
		virtual void onStop(void);
		virtual void onPause(void);
		virtual void onContinue(void);
		virtual void onShutdown(void);

		workerLog_t& getLogger(void);

	signals:
		void started(void);
		void stopped(void);
		void paused(void);
		void resumed(void);
		void shutdown(void);
};

#endif // WORKERSERVICE_H
