#ifndef EVENTNOTIFY_H
#define EVENTNOTIFY_H

#include <Windows.h>

#include <QObject>

#include <memory>
#include <thread>

#include "util.h"
#include "eventlogentry.h"

class eventNotify_t : public QObject
{
	Q_OBJECT

	signals:
		void addRecord(eventLogEntry_t*);
		void errorMessage(const QString&, const QString&, bool);

	public slots:
		void start(void);

	private:
		typedef std::wstring::traits_type::char_type widec_t;

		HANDLE		m_handle;
		HANDLE		m_notify;
		HMODULE		m_dll;
		bool		m_stop;

	protected:
		bool initHandles(void);
		DWORD getLastRecordNumber(DWORD&);
		DWORD seekToLastRecord(void);
		DWORD readRecord(PBYTE&, DWORD, DWORD);
		DWORD dumpNewRecords(void);

	public:
		eventNotify_t(QObject* p);
		~eventNotify_t(void);
		void stop(void);

	
};

#endif // EVENTNOTIFY_H
