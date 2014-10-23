#include "controllerlog.h"


controllerLog_t::controllerLog_t(QObject* p) : eventLog_t("Application", "WINHTTPD-Controller", p)
{
	return;
}

controllerLog_t::~controllerLog_t(void)
{
	return;
}

void 
controllerLog_t::info(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_INFORMATION_TYPE, c);
	return;
}

void 
controllerLog_t::warning(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_WARNING_TYPE, c);
	return;
}

void 
controllerLog_t::error(QString m, eventlog_cat_t c, bool gle)
{
	if (true == gle) 
		m += " " + this->loadErrorMessage();

	writeEvent(m, EVENTLOG_ERROR_TYPE, c);
	return;
}

void 
controllerLog_t::success(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_SUCCESS, c);
	return;
}

void 
controllerLog_t::auditSuccess(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_SUCCESS, c);
	return;
}

void 
controllerLog_t::auditFailure(QString m, eventlog_cat_t c)
{
	writeEvent(m, EVENTLOG_AUDIT_FAILURE, c);
	return;
}
	
void 
controllerLog_t::infoSlot(QString m)
{
	this->info(m);
	return;
}

void 
controllerLog_t::warningSlot(QString m)
{
	this->warning(m);
	return;
}

void 
controllerLog_t::errorSlot(QString m, bool gle)
{
	this->error(m, eventlog_cat_t::CAT_CONTROLLER, gle);
	return;
}

void 
controllerLog_t::auditSuccessSlot(QString m)
{
	this->auditSuccess(m);
	return;
}

void 
controllerLog_t::auditFailureSlot(QString m)
{
	this->auditFailure(m);
	return;
}
