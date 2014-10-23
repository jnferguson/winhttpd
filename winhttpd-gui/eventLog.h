#ifndef EVENLOG_TYPE_H
#define EVENLOG_TYPE_H

#include <Windows.h>

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QList>
#include <QThread>

#include <QMessageBox>

#include <mutex>
#include <stdexcept>
#include <memory>
#include <thread>

#include "util.h"
#include "eventlogentry.h"
#include "eventnotify.h"

class eventLog_t : public QObject
{
	Q_OBJECT

	public:
		typedef std::wstring::traits_type::char_type widec_t;

	public slots:
		void start(void);
		void recvRecord(eventLogEntry_t*);
		void recvErrorMessage(const QString&, const QString&, bool);
		

	signals:
		void entryReady(eventLogEntry_t*);
		void errorMessage(const QString&, const QString&, bool);

	private:
		QTimer*						m_queueTimer;
		eventNotify_t*				m_enotify;
		QList< eventLogEntry_t* >	m_queue;
		std::mutex					m_queueMutex;
		HANDLE						m_handle;
		HMODULE						m_dll;
		HANDLE						m_notify;
		std::mutex					m_handleMutex;
		std::mutex					m_recordIdMutex;
		std::mutex					m_mutex;
		bool						m_firstStart;

		void queueThread(void);

	protected:
		void getAllEntries(void);
		void getEntries(quint64, quint64);
		void getLogEntry(quint64);
		
		HANDLE getEventLogHandle(void);
		HMODULE getDllHandle(void);
		HANDLE getNotifyHandle(void);

		void destroyHandles(void);
		bool initializeHandles(void);

		void addRecord(eventLogEntry_t*);
		eventLogEntry_t* getRecord(void);
		bool isQueueEmpty(void);
		void destroyQueue(void);

		

		quint64 getNewestRecordId(void);
		quint64 getOldestRecordId(void);


	public:
		eventLog_t(QObject* p = nullptr);
		~eventLog_t(void);
		void clearEventLog(void);
	
};

#endif // EVENLOG_TYPE_H
