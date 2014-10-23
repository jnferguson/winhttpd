#include "workerlog.h"

workerLog_t::workerLog_t(QObject* p) : eventLog_t("Application", "WINHTTPD-Worker", p)
{
	return;
}

workerLog_t::~workerLog_t(void)
{
	return;
}

void 
workerLog_t::info(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_INFORMATION_TYPE, c);
	return;
}

void 
workerLog_t::warning(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_WARNING_TYPE, c);
	return;
}

void 
workerLog_t::error(QString m, eventlog_cat_t c, bool gle)
{
	if (true == gle) 
		m += " " + this->loadErrorMessage();

	writeEvent(m, EVENTLOG_ERROR_TYPE, c);
	return;
}

void 
workerLog_t::success(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_SUCCESS, c);
	return;
}

void 
workerLog_t::auditSuccess(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_SUCCESS, c);
	return;
}

void 
workerLog_t::auditFailure(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_FAILURE, c);
	return;
}
	
void 
workerLog_t::infoSlot(QString m)
{
	this->info(m);
	return;
}

void 
workerLog_t::warningSlot(QString m)
{
	this->warning(m);
	return;
}

void 
workerLog_t::errorSlot(QString m, bool gle)
{
	this->error(m, eventlog_cat_t::CAT_CONTROLLER, gle);
	return;
}

void 
workerLog_t::auditSuccessSlot(QString m)
{
	this->auditSuccess(m);
	return;
}

void 
workerLog_t::auditFailureSlot(QString m)
{
	this->auditFailure(m);
	return;
}

void
workerLog_t::createSource(void)
{
	this->createEventSource();
	return;
}