#ifndef CONTROLLERLOG_H
#define CONTROLLERLOG_H

#define WIN32_LEAN_AND_MEAN

#include "eventlog.h"

class controllerLog_t : protected eventLog_t
{
	Q_OBJECT

	private:
	protected:
	public:
		controllerLog_t(QObject* p);
		~controllerLog_t(void);

		void createSource(void);
		virtual void info(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER);
		virtual void warning(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER);
		virtual void error(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER, bool gle = true);
		virtual void success(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER);
		virtual void auditSuccess(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER);
		virtual void auditFailure(QString, eventlog_cat_t c = eventlog_cat_t::CAT_CONTROLLER);
	
	public slots:
		void infoSlot(QString);
		void warningSlot(QString);
		void errorSlot(QString, bool gle = true);
		void auditSuccessSlot(QString);
		void auditFailureSlot(QString);
};

#endif // CONTROLLERLOG_H
