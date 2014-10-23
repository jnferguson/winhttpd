#ifndef FLATFILE_LOGGER_H
#define FLATFILE_LOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QSettings>

#include <stdexcept>
#include <mutex>
#include <functional>

#include "workerlog.h"
#include "http_handler.h"


namespace http {

	class rotate_timer_t : public QObject
	{
		Q_OBJECT

	private:
		std::mutex					m_mutex;
		QTimer						m_timer;
		std::function< void(void) > m_callback;

	protected:
		protected slots :
			virtual void execute(void);

	public:
		rotate_timer_t(std::function< void(void) >&);
		virtual ~rotate_timer_t(void);
		virtual void reset(void);
		virtual void stop(void);
		virtual void start(void);
	};

	class tlog_handler_t : public handler_t
	{
		
		private:
			std::mutex		m_mutex;
			QString			m_filename;
			QDir			m_dir;
			QFile			m_file;
			bool			m_rotate;
			//QTimer		m_timer;
			rotate_timer_t*	m_time;

		protected:
			bool openLogFile(void);
			bool closeLogFile(void);
			void rotateLogFile(void);

		public:
			tlog_handler_t(workerLog_t&);
			virtual ~tlog_handler_t(void);
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_wptr_t&, http::resp_wptr_t&);
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_sptr_t&, http::resp_sptr_t&);
	};
};

#endif