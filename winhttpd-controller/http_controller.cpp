#include "http_controller.h"

#pragma comment(lib, "httpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

QStringList
http_controller_t::getAllAddresses(void)
{
	QStringList					ret;
	WSAData						data = { 0 };
	ULONG						rval(NO_ERROR);
	ULONG						size(0);
	ULONG						flags(GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST);
	IP_ADAPTER_ADDRESSES*		adapters(nullptr);
	IP_ADAPTER_ADDRESSES*		current(nullptr);
	IP_ADAPTER_UNICAST_ADDRESS*	address(nullptr);

	if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
		m_log.error("Error while initializing WinSock via ::WSAStartup()");
		throw std::runtime_error("Error while initializing WinSock via ::WSAStartup()");
		return ret;
	}

	rval = ::GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, nullptr, &size);

	if (ERROR_BUFFER_OVERFLOW != rval) {
		m_log.error("Error while obtaining network adapters addresses via ::GetAdaptersAddresses()");
		throw std::runtime_error("Error while obtaining network adapters addresses via ::GetAdaptersAddresses()");
		return ret;
	}

	adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(new BYTE[size + 1]);

	rval = ::GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, adapters, &size);

	if (NO_ERROR != rval) {
		m_log.error("Error while obtaining network adapters addresses via ::GetAdaptersAddresses()");
		throw std::runtime_error("Error while obtaining network adapters addresses via ::GetAdaptersAddresses()");
		return ret;
	}

	for (current = adapters; nullptr != current; current = current->Next) {
		for (address = current->FirstUnicastAddress; nullptr != address; address = address->Next) {
			char	buf[4096] = { 0 };
			ULONG	bsize = 4096;

			if (0 != ::WSAAddressToStringA(address->Address.lpSockaddr, address->Address.iSockaddrLength, nullptr, &buf[0], &bsize)) {
				m_log.error("Error while converting IP address to string via ::WSAAddressToString()");
				throw std::runtime_error("Error while converting IP address to string via ::WSAAddressToString()");
				return ret;
			}

			ret.push_back(QString(buf));
		}
	}

	//QList< QNetworkInterface >	ilist;
	//QList< QHostAddress >		hlist;

	//hlist = QNetworkInterface::allAddresses();

	//ilist = qni.allInterfaces();
	
	//ilist = QNetworkInterface::allInterfaces();
	/*
	foreach(QNetworkInterface i, ilist) {
		hlist = i.allAddresses();

		if (QNetworkInterface::InterfaceFlag::IsUp == i.flags())
			foreach(QHostAddress h, hlist)
				ret.push_back(h.toString());
	}

	foreach(QHostAddress h, hlist)
		ret.push_back(h.toString());*/

	delete adapters;
	return ret;

}

bool 
http_controller_t::isAvailableIp(QString& ip)
{
	if (! ip.compare("*"))
		return true;

	foreach(QString hip, m_ips)
		if (! ip.compare(hip))
			return true;

	return false;
}

QStringList 
http_controller_t::getVirtualHostUrls(void)
{
	QSettings				s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	QStringList				vhosts;
	QStringList				binds;
	QString					ip("");

	s.beginGroup("Plugins/VirtualHostManager/Options/Domains");
	vhosts = s.childKeys();
	s.endGroup();

	foreach(QString host, vhosts) {
		bool	https(s.value("Plugins/VirtualHostManager/Options/Domains/" + host + "/HTTPS/Enable", false).toBool());
		bool	http(s.value("Plugins/VirtualHostManager/Options/Domains/" + host + "/HTTP/Enable", false).toBool());
		QString ssl(s.value("Plugins/VirtualHostManager/Options/Domains/" + host + "/HTTPS/Port", "443").toString());
		QString www(s.value("Plugins/VirtualHostManager/Options/Domains/" + host + "/HTTP/Port", "80").toString());

		ip = s.value("Plugins/VirtualHostManager/Options/Domains/" + host + "/BindHosts", "").toString();

		if (ip.isEmpty())
			continue;

		if (false == isAvailableIp(ip)) {
			m_log.error("http_controller_t::getVirtualHostUrls(): A request to bind to an IP (" + ip + ") was made, however that IP is not presently bound to any available interfaces", eventlog_cat_t::CAT_CONTROLLER, false);
			throw std::runtime_error("http_controller_t::getVirtualHostUrls(): A request to bind to an IP was made, however that IP is not presently bound to any available interfaces");
		}

		if (true == https)
			binds.push_back("https://" + ip + ":" + ssl + "/");
		if (true == http)
			binds.push_back("http://" + ip + ":" + www + "/");
	}

	binds.removeDuplicates();
	return binds;
}

QStringList 
http_controller_t::getBaseHostUrls(void)
{
	QSettings				s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	QStringList				base(s.value("Controller/BindHosts", "").toString().split(',', QString::SplitBehavior::SkipEmptyParts));
	QStringList				binds;

	foreach(QString host, base) {
		bool	https(s.value("Controller/HTTPS/Enable", false).toBool());
		bool	http(s.value("Controller/HTTP/Enable", true).toBool());
		QString ssl(s.value("Controller/HTTPS/Port", "443").toString());
		QString www(s.value("Controller/HTTP/Port", "80").toString());

		if (host.isEmpty())
			continue;

		if (false == isAvailableIp(host)) {
			m_log.error("http_controller_t::getBaseHostUrls(): A request to bind to an IP (" + host + ") was made, however that IP is not presently bound to any available interfaces", eventlog_cat_t::CAT_CONTROLLER, false);
			throw std::runtime_error("http_controller_t::getBaseHostUrls(): A request to bind to an IP was made, however that IP is not presently bound to any available interfaces");
		}

		if (true == https)
			binds.push_back("https://" + host + ":" + ssl + "/");
		if (true == http)
			binds.push_back("http://" + host + ":" + www + "/");
	}

	binds.removeDuplicates();
	return binds;
}

QString
http_controller_t::getManagementUrl(void)
{
	QSettings				s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	bool					manage(s.value("Controller/ManagementURI/Enable", false).toBool());
	QString					mproto(s.value("Controller/ManagementURI/Protocol", "HTTPS").toString());
	QString					mhost(s.value("Controller/ManagementURI/Host", "127.0.0.1").toString());
	QString					mport(s.value("Controller/ManagementURI/Port", "443").toString());
	QString					muri(s.value("Controller/ManagementURI/Path", "/manage").toString());
	QString					ret("");

	if (false == manage)
		return ret;

	if (false == isAvailableIp(mhost)) {
		m_log.error("http_controller_t::getManagementUrl(): A request to bind to an IP (" + mhost + ") was made, however that IP is not presently bound to any available interfaces", eventlog_cat_t::CAT_CONTROLLER, false);
		throw std::runtime_error("http_controller_t::getManagementUrl(): A request to bind to an IP was made, however that IP is not presently bound to any available interfaces");
	}

	if (!mproto.compare("https", Qt::CaseInsensitive))
		return QString("https://" + mhost + ":" + mport + muri);
	else
		return QString("http://" + mhost + ":" + mport + muri);
}

bool
http_controller_t::initDefaultEndpoints(void)
{
	ULONG					r(NO_ERROR);
	QString					m("");
	QStringList				b;

	try {
		
		b = getVirtualHostUrls();
		b += getBaseHostUrls();
		m = getManagementUrl();

		if (!m.isEmpty())
			b.push_back(m);

	} catch (std::runtime_error&) {
		return false;
	}

	b.removeDuplicates();

	// XXX JF FIXME - fix code to only bind to IPs that aren't already covered in
	// a wildcard bind.
	// FIXME - make weak and strong binds work as expected...
	foreach(QString u, b) {
		m_log.info(QString("Adding " + u + " to URL group.."));

		r = ::HttpAddUrlToUrlGroup(m_group, u.toStdWString().c_str(), NULL, 0);

		if (NO_ERROR != r) {
			::SetLastError(r);

			if (ERROR_INVALID_PARAMETER == r)
				m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + u + "), received ERROR_INVALID_PARAMETER, which can occur when the port is already in use", eventlog_cat_t::CAT_CONTROLLER, false);
			else
				m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + u + "), error number: " + QString::number(r));

			return false;
		}
	}

	::SetLastError(NO_ERROR);
	return true;
}

/*bool 
http_controller_t::initDefaultEndpoints(void)
{
	QSettings		s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	ULONG			r		= NO_ERROR;
	bool			wc		= (0 == s.value("General/InterpretWeakBindWildcardAsStrong", "false").toString().compare("true", Qt::CaseInsensitive) ? true : false);
	QString			manage	= s.value("Controller/ManagementURI/Enable", "").toString();
	QString			https	= s.value("Controller/HTTPS/Enable", "").toString();
	QString			http	= s.value("Controller/HTTP/Enable", "").toString();
	QStringList		binds	= s.value("Controller/BindHosts", "").toString().split(',', QString::SplitBehavior::SkipEmptyParts);
	QString			port	= "";
	QString			tmp		= "";
	bool			okay	= false;
	
	
	if (! https.compare("true", Qt::CaseInsensitive)) {
		for (auto itr = binds.begin(); itr != binds.end(); itr++) {
			port = s.value("Controller/HTTPS/Port", "443").toString();

			if (! itr->compare("*") && true == wc) 
				tmp = "https://+:" + port + "/";
			else 
				tmp = "https://" + *itr + ":" + port + "/";

			m_log.info(QString("Adding " + tmp + " to URL group.."));

			r = ::HttpAddUrlToUrlGroup(m_group, tmp.toStdWString().c_str(), NULL, 0);

			if (NO_ERROR != r) {
				::SetLastError(r);

				if (ERROR_INVALID_PARAMETER == r)
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp +"), received ERROR_INVALID_PARAMETER, which can occur when the port is already in use", eventlog_cat_t::CAT_CONTROLLER, false);
				else 
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp + "), error number: " + QString::number(r));

				return false;
			}
		}
	}

	if (! http.compare("true", Qt::CaseInsensitive)) {
		for (auto itr = binds.begin(); itr != binds.end(); itr++) {
			port = s.value("Controller/HTTP/Port", "80").toString();

			if (! itr->compare("*") && true == wc) 
				tmp = "http://+:" + port + "/";
			else 
				tmp = "http://" + *itr + ":" + port + "/";

			m_log.info(QString("Adding " + tmp + " to URL group.."));

			r = ::HttpAddUrlToUrlGroup(m_group, tmp.toStdWString().c_str(), NULL, 0);

			if (NO_ERROR != r) {
				::SetLastError(r);

				if (ERROR_INVALID_PARAMETER == r)
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp + "), received ERROR_INVALID_PARAMETER, which can occur when the port is already in use", eventlog_cat_t::CAT_CONTROLLER, false);
				else 
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp + "), error number: " + QString::number(r));

				return false;
			}
		}
	}

	if (! manage.compare("true", Qt::CaseInsensitive)) {
			QString	mproto	= s.value("Controller/ManagementURI/Protocol", "HTTPS").toString();
			QString mhost	= s.value("Controller/ManagementURI/Host", "127.0.0.1").toString();
			QString mport	= s.value("Controller/ManagementURI/Port", "443").toString();
			QString muri	= s.value("Controller/ManagementURI/Path", "/manage").toString();

			if (! mproto.compare("https", Qt::CaseInsensitive)) 
				tmp = "https://" + mhost + ":" + mport + muri + "/";
			else 
				tmp = "http://" + mhost + ":" + mport + muri + "/";

			m_log.info("Adding " + tmp + " to URL group for management URI");

			r = ::HttpAddUrlToUrlGroup(m_group, tmp.toStdWString().c_str(), NULL, 0);

			if (NO_ERROR != r) {
				::SetLastError(r);

				if (ERROR_INVALID_PARAMETER == r)
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp + "), received ERROR_INVALID_PARAMETER, which can occur when the port is already in use", eventlog_cat_t::CAT_CONTROLLER, false);
				else 
					m_log.error("Error while calling ::HttpAddUrlToURlGroup(" + tmp + "), error number: " + QString::number(r));

				return false;
			}
	}*/
/*
		s.setValue("Controller/EnableManagementURI", "true");
		s.setValue("Controller/ManagementURIHost", "127.0.0.1");
		s.setValue("Controller/ManagementURIPath", "/manage");
*/
//		r = ::HttpAddUrlToUrlGroup(m_group, L"https://+:443/", NULL, 0);

	/*
	r = ::HttpAddUrlToUrlGroup(m_group, L"http://+:80/", NULL, 0);

	if (NO_ERROR != r) {
		::SetLastError(r);

		if (ERROR_INVALID_PARAMETER == r)
			m_log.error("Error while calling ::HttpAddUrlToURlGroup(http://+:80/), received ERROR_INVALID_PARAMETER, which can occur when the port is already in use", eventlog_cat_t::CAT_CONTROLLER, false);
		else 
			m_log.error("Error while calling ::HttpAddUrlToURlGroup(http://+:80/), error number: " + QString::number(r));

		return false;
	}*/
/*
	::SetLastError(NO_ERROR);
	return true;

}*/

PSECURITY_ATTRIBUTES 
http_controller_t::getSecurityAttributes(void)
{
	PACL					acl		= nullptr;
	PSID					nsid	= nullptr;
	PSID					ssid	= nullptr;
	PSECURITY_DESCRIPTOR	sd		= nullptr;
	PSECURITY_ATTRIBUTES	sa		= nullptr;
	DWORD					asz		= 0;

	sd = (PSECURITY_DESCRIPTOR)new BYTE[sizeof(SECURITY_DESCRIPTOR)];
	::memset(sd, 0, sizeof(SECURITY_DESCRIPTOR));

	sa = (PSECURITY_ATTRIBUTES)new BYTE[sizeof(SECURITY_ATTRIBUTES)];
	::memset(sa, 0, sizeof(SECURITY_ATTRIBUTES));

	if (! ::ConvertStringSidToSid(L"S-1-5-20", &nsid)) { // networkservice
		m_log.error("Error while calling ConvertStringSidToSid()");
		return nullptr;
	}

	if (! ::IsValidSid(nsid)) {
		m_log.error("Error while calling IsValidSid()");
		return nullptr;
	}

	if (! ::ConvertStringSidToSid(L"S-1-5-18", &ssid)) { // local system
		m_log.error("Error while calling ConvertStringSidToSid()");
		return nullptr;
	}
			 
	if (! ::IsValidSid(ssid)) {
		m_log.error("Error while calling IsValidSid()");
		return nullptr;
	}

	asz = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 2 - sizeof(DWORD) * 2;
	asz += ::GetLengthSid(nsid);
	asz += ::GetLengthSid(ssid);

	acl = (PACL)new BYTE[asz];
	::memset(acl, 0, asz);

	if (FALSE == ::InitializeAcl(acl, asz, ACL_REVISION_DS)) {
		m_log.error("Error while calling InitializeAcl()");
		return nullptr;
	}

	if (FALSE == ::AddAccessAllowedAce(acl, ACL_REVISION_DS, GENERIC_READ, nsid)) {
		m_log.error("Error while calling AddAccessAllowedAce()");
		return nullptr;
	}

	if (FALSE == ::AddAccessAllowedAce(acl, ACL_REVISION_DS, GENERIC_READ, ssid)) {
		m_log.error("Error while calling AddAccessAllowedAce()");
		return nullptr;
	}

	if (FALSE == ::InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
		m_log.error("Error while calling InitializeSecurityDescriptor()");
		return nullptr;
	}

	if (FALSE == ::SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE)) {
		m_log.error("Error while calling SetSecurityDescriptorDacl()");
		return nullptr;
	}

			
	sa->nLength					= sizeof(SECURITY_ATTRIBUTES);
	sa->bInheritHandle			= FALSE;
	sa->lpSecurityDescriptor	= sd;

	return sa;
}


http_controller_t::http_controller_t(controllerLog_t& log, ULONG len) : m_log(log), m_rqueue(nullptr), m_session(0), m_group(0)
{
	std::lock_guard< std::mutex >		l(m_mutex);
	m_ips = getAllAddresses();

	return;
}

http_controller_t::~http_controller_t(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	if (nullptr != m_rqueue) {
		::HttpCloseUrlGroup(m_group);
		::HttpCloseRequestQueue(m_rqueue);
		::HttpCloseServerSession(m_session);
		::HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
		::HttpTerminate(HTTP_INITIALIZE_CONFIG, nullptr);

		m_group			= 0;
		m_session		= 0;
		m_rqueue		= nullptr;

		::SetLastError(NO_ERROR);	
	}

	return;
}

HTTP_503_RESPONSE_VERBOSITY 
http_controller_t::get503Verbosity(void)
{
	QSettings		s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	QString			v(s.value("Controller/Queue/504ResponseVerbosity", "Basic").toString());

	if (! v.compare("full", Qt::CaseInsensitive))
		return HTTP_503_RESPONSE_VERBOSITY::Http503ResponseVerbosityFull;
	else if (! v.compare("limited", Qt::CaseInsensitive))
		return HTTP_503_RESPONSE_VERBOSITY::Http503ResponseVerbosityLimited;
	
	return HTTP_503_RESPONSE_VERBOSITY::Http503ResponseVerbosityBasic;
}

ULONG 
http_controller_t::getQueueLength(ULONG l)
{
	QSettings		s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	bool			o = false;
	unsigned int	v = s.value("Controller/Queue/Length", QString::number(l)).toUInt(&o);

	if (false == o)
		return l;

	return static_cast< ULONG >(v);
}

bool
http_controller_t::start(ULONG len)
{
	std::lock_guard< std::mutex >		l(m_mutex);
	const HTTPAPI_VERSION				v = HTTPAPI_VERSION_2;
	ULONG								r = NO_ERROR;
	const HTTP_503_RESPONSE_VERBOSITY	rv = get503Verbosity(); //HTTP_503_RESPONSE_VERBOSITY::Http503ResponseVerbosityBasic;
	ULONG								ql = getQueueLength(len);
	const HTTP_ENABLED_STATE			di = HttpEnabledStateActive; //HttpEnabledStateInactive;
	PSECURITY_ATTRIBUTES				sa = getSecurityAttributes();
	HTTP_BINDING_INFO					bi = {0};
	HTTP_PROPERTY_FLAGS					fl = {0};

	if (nullptr == sa) 
		return false;

	r = ::HttpInitialize(v, HTTP_INITIALIZE_SERVER, nullptr);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling HttpInitialize()");
		return false;
	}

	r = ::HttpCreateServerSession(v, &m_session, 0);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpCreateServerSession()");
		return false;
	}

	r = ::HttpCreateRequestQueue(v, L"winhttpd", sa, HTTP_CREATE_REQUEST_QUEUE_FLAG_CONTROLLER, &m_rqueue);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpCreateRequestQueue()");
		::HttpCloseServerSession(m_session);
		return false;
	}

	r = ::HttpSetRequestQueueProperty(m_rqueue, HttpServer503VerbosityProperty, (PVOID)&rv, sizeof(HTTP_503_RESPONSE_VERBOSITY), 0, nullptr);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while setting server 503 verbosity property");
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);	
		return false;
	}

	r = ::HttpSetRequestQueueProperty(m_rqueue, HttpServerQueueLengthProperty, (PVOID)&ql, sizeof(ql), 0, nullptr);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpSetRequestQueueProperty()");
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);
		return false;
	}

	r = ::HttpSetRequestQueueProperty(m_rqueue, HttpServerStateProperty, (PVOID)&di, sizeof(HTTP_ENABLED_STATE), 0, nullptr);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpSetRequestQueueProperty()");
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);
		return false;
	}

	r = ::HttpCreateUrlGroup(m_session, &m_group, 0);

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpCreateUrlGroup()");
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);
		return false;
	}

	fl.Present = TRUE;
	bi.RequestQueueHandle = m_rqueue;
	bi.Flags = fl;

	r = ::HttpSetUrlGroupProperty(m_group, HttpServerBindingProperty, &bi, sizeof(HTTP_BINDING_INFO));

	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while calling ::HttpSetUrlGroupProperty()");
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);
		::HttpCloseUrlGroup(m_group);
		return false;
	}

	if (false == initDefaultEndpoints()) { 
		::HttpCloseServerSession(m_session);
		::HttpCloseRequestQueue(m_rqueue);
		::HttpCloseUrlGroup(m_group);
		return false;
	}

	::SetLastError(NO_ERROR);
	return true;
}

bool
http_controller_t::stop(void)
{
	std::lock_guard< std::mutex >		l(m_mutex);

	if (nullptr != m_rqueue) {
		::HttpCloseUrlGroup(m_group);
		::HttpCloseRequestQueue(m_rqueue);
		::HttpCloseServerSession(m_session);
		::HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
		::HttpTerminate(HTTP_INITIALIZE_CONFIG, nullptr);

		m_group			= 0;
		m_session		= 0;
		m_rqueue		= nullptr;

		::SetLastError(NO_ERROR);	
	}

	return true;
}

bool 
http_controller_t::addUrl(std::string& s, unsigned long long c)
{
	return addUrl(std::wstring(s.begin(), s.end()), c);
}

bool 
http_controller_t::addUrl(const char* s, unsigned long long c)
{
	if (nullptr == s)
		return false;

	return addUrl(std::string(s), c);
}

bool
http_controller_t::addUrl(const wchar_t* s, unsigned long long c)
{
	if (nullptr == s)
		return false;

	return addUrl(std::wstring(s), c);
}

bool
http_controller_t::addUrl(std::wstring& s, unsigned long long c)
{
	ULONG r = NO_ERROR;

	r = ::HttpAddUrlToUrlGroup(m_group, s.c_str(), c, 0);
			
	if (NO_ERROR != r) {
		::SetLastError(r);
		m_log.error("Error while adding Url via HttpAddUrlToUrlGroup(), received error number " + QString::number(r));
		return false;
	}

	::SetLastError(NO_ERROR);
	return true;
}


bool
http_controller_t::waitForRequest(void)
{
	::HttpWaitForDemandStart(m_rqueue, nullptr);
	return true;
}

/*bool
http_t::isServerActive(void)
{
	ULONG				r	= NO_ERROR;
	HTTP_STATE_INFO		si	= {0};
	ULONG				len = 0;

	r = ::HttpQueryRequestQueueProperty(m_rqueue, HttpServerStateProperty, &si, sizeof(HTTP_STATE_INFO), 0, &len, nullptr);

	if (NO_ERROR != r) {
		m_log.error("http_t::isServerActive(): HttpQueryRequestQueueProperty() error ");
		return false;
	}

	if (HttpEnabledStateActive == si.State)
		return true;

	return false;
}*/

/*#define EA_HTTP_SA_SIZE 2

bool 
http_t::createWorkerEvent(void)
{
	PACL					acl		= nullptr;
	PSID					nsid	= nullptr;
	PSID					ssid	= nullptr;
	PSECURITY_DESCRIPTOR	sd		= nullptr;
	PSECURITY_ATTRIBUTES	sa		= nullptr;
	HANDLE					wEvent	= nullptr;
	DWORD					asz		= 0;

	sd = (PSECURITY_DESCRIPTOR)new BYTE[sizeof(SECURITY_DESCRIPTOR)];
	::memset(sd, 0, sizeof(SECURITY_DESCRIPTOR));

	sa = (PSECURITY_ATTRIBUTES)new BYTE[sizeof(SECURITY_ATTRIBUTES)];
	::memset(sa, 0, sizeof(SECURITY_ATTRIBUTES));

	if (! ::ConvertStringSidToSid(L"S-1-5-20", &nsid)) { // networkservice
		m_log.error("http_t::createWorkerEvent(): error while calling ConvertStringSidToSid()");
		delete sd;
		delete sa;
		return false;
	}

	if (! ::IsValidSid(nsid)) {
		m_log.error("http_t::createWorkerEvent(): error while calling IsValidSid()");
		delete sd;
		delete sa;
		::FreeSid(nsid);
		return false;
	}

	if (! ::ConvertStringSidToSid(L"S-1-5-18", &ssid)) { // local system
		m_log.error("http_t::createWorkerEvent(): error while calling ConvertStringSidToSid()");
		delete sd;
		delete sa;
		::FreeSid(nsid);
		return false;
	}
			 
	if (! ::IsValidSid(ssid)) {
		m_log.error("http_t::createWorkerEvent(): error while calling IsValidSid()");
		delete sd;
		delete sa;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	asz = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 2 - sizeof(DWORD) * 2;
	asz += ::GetLengthSid(nsid);
	asz += ::GetLengthSid(ssid);

	acl = (PACL)new BYTE[asz];
	::memset(acl, 0, asz);

	if (FALSE == ::InitializeAcl(acl, asz, ACL_REVISION_DS)) {
		m_log.error("http_t::createWorkerEvent(): error while calling InitializeAcl()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	if (FALSE == ::AddAccessAllowedAce(acl, ACL_REVISION_DS, EVENT_ALL_ACCESS|EVENT_MODIFY_STATE, nsid)) {
		m_log.error("http_t::createWorkerEvent(): error while calling AddAccessAllowedAce()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	if (FALSE == ::AddAccessAllowedAce(acl, ACL_REVISION_DS, EVENT_ALL_ACCESS|EVENT_MODIFY_STATE, ssid)) {
		m_log.error("http_t::createWorkerEvent(): error while calling AddAccessAllowedAce()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	if (FALSE == ::InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
		m_log.error("http_t::createWorkerEvent(): error while calling InitializeSecurityDescriptor()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	if (FALSE == ::SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE)) {
		m_log.error("http_t::createWorkerEvent(): error while calling SetSecurityDescriptorDacl()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

			
	sa->nLength					= sizeof(SECURITY_ATTRIBUTES);
	sa->bInheritHandle			= FALSE;
	sa->lpSecurityDescriptor	= sd;

	m_cevent = ::CreateEvent(sa, TRUE, FALSE, L"Global\\WINHTTPD_Worker_Notification_Event");

	if (nullptr == m_cevent) {
		m_log.error("http_t::createWorkerEvent(): error while calling CreateEvent()");
		delete sd;
		delete sa;
		delete acl;
		::FreeSid(nsid);
		::FreeSid(ssid);
		return false;
	}

	delete sd;
	delete sa;
	delete acl;
	::FreeSid(nsid);
	::FreeSid(ssid);
	return true;
}*/


