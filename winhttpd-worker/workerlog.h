#ifndef WORKERLOG_H
#define WORKERLOG_H

#define WIN32_LEAN_AND_MEAN

#include <QObject>
#include <QString>
#include "eventlog.h"

class workerLog_t : protected eventLog_t
{
	Q_OBJECT

	private:
	protected:
	public:
		workerLog_t(QObject* p = nullptr);
		~workerLog_t(void);

		virtual void info(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER);
		virtual void warning(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER);
		virtual void error(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER, bool gle = true);
		virtual void success(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER);
		virtual void auditSuccess(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER);
		virtual void auditFailure(QString, eventlog_cat_t c = eventlog_cat_t::CAT_WORKER);

		void createSource(void);
	
	public slots:
		void infoSlot(QString);
		void warningSlot(QString);
		void errorSlot(QString, bool gle = true);
		void auditSuccessSlot(QString);
		void auditFailureSlot(QString);

	
};

#endif // WORKERLOG_H
