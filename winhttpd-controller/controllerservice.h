#ifndef CONTROLLERSERVICE_H
#define CONTROLLERSERVICE_H

#include <QCoreApplication>
#include <QThread>

#include <mutex>
#include <thread>
#include <memory>

#include "qservice.h"
#include "controllerlog.h"
#include "http_controller.h"



class controllerService_t : public qservice_t
{
	Q_OBJECT

	private:
		qservice_t::qservice_state_t			m_state;
		std::unique_ptr< std::thread >			m_thread;
		std::unique_ptr< controllerLog_t >		m_log;
		std::unique_ptr< http_controller_t >	m_http;
		std::mutex								m_mutex;

	protected:
		virtual void setState(qservice_state_t);
		virtual qservice_t::qservice_state_t getState(void);
		virtual void fsm(void);
		void processRequests(void);

		//bool isStopped(void) { return m_stop == true; }
		//void setStop(void) { m_stop = true; return; }

	public:
		controllerService_t(QString, QString, QString, QString, QObject* p = nullptr);
		~controllerService_t(void);

		virtual void onStart(DWORD, LPTSTR*);
		virtual void onStop(void);
		virtual void onPause(void);
		virtual void onContinue(void);
		virtual void onShutdown(void);

	signals:
		void started(void);
		void stopped(void);
		void paused(void);
		void resumed(void);
		void shutdown(void);
};

#endif // CONTROLLERSERVICE_H
