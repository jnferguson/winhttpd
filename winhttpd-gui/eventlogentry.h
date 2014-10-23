#ifndef EVENTLOGENTRY_H
#define EVENTLOGENTRY_H

#include <Windows.h>

#include <QObject>
#include <QString>
#include <QDateTime>

#include <string>
#include <memory>

#include "util.h"

class eventLogEntry_t : public QObject
{
	Q_OBJECT

	public:
		typedef std::shared_ptr< eventLogEntry_t > log_ptr_t;

	private:
		typedef std::wstring::traits_type::char_type widec_t;

		HMODULE		m_dll;
		quint64		m_id;
		QString		m_type;
		QString		m_message;
		QString		m_category;
		QDateTime	m_timestamp;

	protected:
		QString getCategory(DWORD);
		QString getType(WORD);
		QString getEventMessage(EVENTLOGRECORD&);
		QString getEventData(EVENTLOGRECORD&);

	public:
		eventLogEntry_t(EVENTLOGRECORD&);
		~eventLogEntry_t(void);

		quint64 getId(void) { return m_id; }
		QString getType(void) { return m_type; }
		QString getMessage(void) { return m_message; }
		QString getTimeStamp(void) { return m_timestamp.toString("yyyy-dd-MMM hh:mm:ss"); }
		QString getCategory(void) { return m_category; }
};

#endif // EVENTLOGENTRY_H
