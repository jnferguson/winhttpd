#ifndef HTTPTYPE_H
#define HTTPTYPE_H

#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>
#include <Sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#include <stdexcept>
#include <mutex>
#include <vector>
#include <string>

#include <QSettings>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

#include "controllerlog.h"

class http_controller_t
{
	private:
		std::mutex					m_mutex;
		HANDLE						m_rqueue;
		HANDLE						m_cevent;
		HTTP_SERVER_SESSION_ID		m_session;
		HTTP_URL_GROUP_ID			m_group;
		controllerLog_t&			m_log;
		QStringList					m_ips;
		//bool						m_isshutdown;

	protected:
		LPSECURITY_ATTRIBUTES getSecurityAttributes(void);
		QStringList getAllAddresses(void);
		QStringList getVirtualHostUrls(void);
		QStringList getBaseHostUrls(void);
		QString		getManagementUrl(void);

		bool isAvailableIp(QString&);

		bool initDefaultEndpoints(void);
		//bool initEndpoints(void);
		HTTP_503_RESPONSE_VERBOSITY get503Verbosity(void);
		ULONG getQueueLength(ULONG);

		//bool createWorkerEvent(void);

	public:
		http_controller_t(controllerLog_t& log, ULONG len = 4096);
		~http_controller_t(void);

		bool waitForRequest(void);

		bool addUrl(std::string& s, unsigned long long c = 0);
		bool addUrl(const char* s, unsigned long long c = 0);
		bool addUrl(const wchar_t* s, unsigned long long c = 0);
		bool addUrl(std::wstring& s, unsigned long long c = 0);

		bool start(ULONG len = 4096);
		bool stop(void);
		//void shutdown(void);
		//bool isServerActive(void);
};



#endif // HTTPTYPE_H
