#ifndef EVENTLOG_H
#define EVENTLOG_H

#define WIN32_LEAN_AND_MEAN
#include <QObject>
#include <QString>
#include <QSettings>
#include <QStringList>
#include <QCoreApplication>

#include <string>
#include <mutex>

#include <Windows.h>

#include "winhttpd_messages.h"


typedef enum { CAT_CONTROLLER = CONTROLLER_CATEGORY, CAT_WORKER = WORKER_CATEGORY, CAT_SERVICE = SERVICE_CATEGORY } eventlog_cat_t;

class eventLog_t : public QObject
{
	Q_OBJECT

	private:
		QString				m_name;
		QString				m_source;
		HANDLE				m_handle;
		std::mutex			m_mutex;

		eventLog_t(void) {}

	protected:
		static QString loadMessage(DWORD, ...);
		static QString loadErrorMessage(DWORD e = ::GetLastError());
		void writeEvent(const QString&, const WORD, eventlog_cat_t c);
		void createEventSource(void);
		void deleteEventSource(void);


	public:
		eventLog_t(QString, QString, QObject *p = nullptr);
		~eventLog_t(void);

		inline const TCHAR* const log(void) const
		{
			return m_name.toStdWString().c_str();
		}
		inline const TCHAR* const source(void) const
		{
			return m_source.toStdWString().c_str();
		}

		virtual void info(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE);
		virtual void warning(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE);
		virtual void error(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE, bool gle = true);
		virtual void success(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE);
		virtual void auditSuccess(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE);
		virtual void auditFailure(QString, eventlog_cat_t c = eventlog_cat_t::CAT_SERVICE);

		void clear(void);
		DWORD numberOfEventLogEntries(void);
};

#endif // EVENTLOG_H
