#include "http_req.h"

#pragma comment(lib, "Ws2_32.lib")

namespace http {

	req_t::req_t(HTTP_REQUEST* r, workerLog_t& log) : m_log(log), m_headers(log)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		if (nullptr == r)
			throw std::runtime_error("http::req_t::req_t(): invalid parameter specified (nullptr)");

		m_id = r->RequestId;
		m_connectId = r->ConnectionId;
		m_totalLength = r->BytesReceived;
		m_rawConnectId = r->RawConnectionId;
		m_version.MajorVersion = r->Version.MajorVersion;
		m_version.MinorVersion = r->Version.MinorVersion;

		initializeVerb(*r);
		initializeUrl(*r);
		initializeAddress(*r);
		initializeHeaders(*r);
		initializeBody(*r);
		//initializeSsl(*r);
		return;

	}

	req_t::~req_t(void)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		m_headers.clearHeaders();
		m_body.clear();
		m_id = HTTP_NULL_ID;

		return;
	}

	void
	req_t::initializeVerb(HTTP_REQUEST& r)
	{
		m_verb = r.Verb;

		if (HTTP_VERB::HttpVerbUnknown == m_verb) {
			if (nullptr == r.pUnknownVerb || 0 == r.UnknownVerbLength)
				throw std::runtime_error("http::req_t::initializeVerb(): malformed HTTP_REQUEST received (unknown verb)");

			m_unknownVerb = QString::fromUtf8(reinterpret_cast<const char*>(r.pUnknownVerb), r.UnknownVerbLength);
		}

		return;
	}

	void
	req_t::initializeUrl(HTTP_REQUEST& r)
	{

		if (nullptr == r.pRawUrl || 0 == r.RawUrlLength)
			throw std::runtime_error("http::req_t::initializeUrl(): malformed HTTP_REQUEST received (raw URL)");

		m_rawUrl = QString::fromUtf8(r.pRawUrl, r.RawUrlLength);

		if (0 == r.CookedUrl.AbsPathLength || 0 == r.CookedUrl.FullUrlLength)
			throw std::runtime_error("http::req_t::initializeUrl(): malformed HTTP_REQUEST received (abspath or fullurl)");

		m_path = QString::fromWCharArray(r.CookedUrl.pAbsPath);
		m_url = QString::fromWCharArray(r.CookedUrl.pFullUrl);

		if (0 == r.CookedUrl.HostLength) {
			if (0 == m_version.MajorVersion && 9 >= m_version.MinorVersion)
				m_host = "";
			else
				throw std::runtime_error("http::req_t::initializeUrl(): malformed HTTP_REQUEST received (host)");
		}
		else
			m_host = QString::fromWCharArray(r.CookedUrl.pHost);

		if (0 != r.CookedUrl.QueryStringLength)
			m_query = QString::fromWCharArray(r.CookedUrl.pQueryString);
		else
			m_query = "";

		return;
	}

	void
	req_t::initializeAddress(HTTP_REQUEST& r)
	{
		if (nullptr == r.Address.pLocalAddress || nullptr == r.Address.pRemoteAddress)
			throw std::runtime_error("http::req_t::initializeAddress(): malformed HTTP_REQUEST received (address)");

		m_laddr = addressToString(*r.Address.pLocalAddress);
		m_lport = portToString(*r.Address.pLocalAddress);
		m_raddr = addressToString(*r.Address.pRemoteAddress);
		m_rport = portToString(*r.Address.pRemoteAddress);
		return;
	}

	void
	req_t::initializeHeaders(HTTP_REQUEST& r)
	{
		for (auto idx = 0; idx < HTTP_HEADER_ID::HttpHeaderRequestMaximum; idx++)
			if (0 != r.Headers.KnownHeaders[idx].RawValueLength && nullptr != r.Headers.KnownHeaders[idx].pRawValue)
				m_headers.addHeader(static_cast<HTTP_HEADER_ID>(idx), r.Headers.KnownHeaders[idx].pRawValue);

		for (auto idx = 0; idx < r.Headers.UnknownHeaderCount; idx++)
			if (0 != r.Headers.pUnknownHeaders[idx].NameLength && nullptr != r.Headers.pUnknownHeaders[idx].pName)
				if (0 != r.Headers.pUnknownHeaders[idx].RawValueLength && nullptr != r.Headers.pUnknownHeaders[idx].pRawValue)
					m_headers.addHeader(r.Headers.pUnknownHeaders[idx].pName, r.Headers.pUnknownHeaders[idx].pRawValue);

		return;
	}

	void
	req_t::initializeBody(HTTP_REQUEST& r)
	{
		if (0 != r.EntityChunkCount && nullptr != r.pEntityChunks) {
			for (auto cnt = 0; cnt < r.EntityChunkCount; cnt++) {
				switch (r.pEntityChunks[cnt].DataChunkType) {
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromMemory:
					m_body.append(static_cast<char*>(r.pEntityChunks[cnt].FromMemory.pBuffer), r.pEntityChunks[cnt].FromMemory.BufferLength);
					break;
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFileHandle:			// fallthrough
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFragmentCache:		// unhandled
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFragmentCacheEx:	// at present
				default:
					throw std::runtime_error("http::req_t::initializeBody(): Unsupported HTTP data chunk type received");
				}
			}
		}

		return;
	}

	void
	req_t::initializeSsl(HTTP_REQUEST& r)
	{
		/*if (nullptr != r.pSslInfo) {
			m_ssl.hasClientCertificate	= (r.pSslInfo->SslClientCertNegotiated != 0 ? true : false);
			m_ssl.svr.certSize			= r.pSslInfo->ServerCertKeySize;
			m_ssl.svr.keySize			= r.pSslInfo->ConnectionKeySize;

			if (0 != r.pSslInfo->ServerCertIssuerSize && nullptr != r.pSslInfo->pServerCertIssuer)
			m_ssl.svr.issuer	= QString::fromUtf8(reinterpret_cast< const char* >(r.pSslInfo->pServerCertIssuer), r->pSslInfo->ServerCertIssuerSize);
			// else throw ?

			if (0 != r.pSslInfo->ServerCertIssuerSize && nullptr != r.pSslInfo->pServerCertSubject)
			m_ssl.svr.subject	=  QString::fromUtf8(reinterpret_cast< const char* >(r.pSslInfo->pServerCertSubject), r->pSslInfo->ServerCertSubjectSize);
			// else throw?

			if (true == m_ssl.hasClientCertificate && nullptr != r.pSslInfo->pClientCertInfo) {
			m_ssl.client.flags			= r.pSslInfo->pClientCertInfo->CertFlags;
			m_ssl.client.certificate	= QString::fromUtf8(reinterpret_cast< const char* >(r.pSslInfo->pClientCertInfo->pCertEncoded), r->pSslInfo->pClientCertInfo->CertEncodedSize);

			} else if (true == m_ssl.hasClientCertificate && nullptr == r.pSslInfo->pClientCertInfo)
			throw std::runtime_error("http::req_t::initializeSsl(): Invalid SSL state encountered");
			}*/

		return;
	}

	http::id_t
	req_t::getId(void)
	{
		return m_id;
	}

	QString
	req_t::getVerbString(void)
	{
		QString r;

		switch (m_verb) {
		case HTTP_VERB::HttpVerbCONNECT:
			r = "CONNECT";
			break;

		case HTTP_VERB::HttpVerbCOPY:
			r = "COPY";
			break;

		case HTTP_VERB::HttpVerbDELETE:
			r = "DELETE";
			break;

		case HTTP_VERB::HttpVerbGET:
			r = "GET";
			break;

		case HTTP_VERB::HttpVerbHEAD:
			r = "HEAD";
			break;

		case HTTP_VERB::HttpVerbLOCK:
			r = "LOCK";
			break;

		case HTTP_VERB::HttpVerbMOVE:
			r = "MOVE";
			break;

		case HTTP_VERB::HttpVerbMKCOL:
			r = "MKCOL";
			break;

		case HTTP_VERB::HttpVerbOPTIONS:
			r = "OPTIONS";
			break;

		case HTTP_VERB::HttpVerbPOST:
			r = "POST";
			break;

		case HTTP_VERB::HttpVerbPROPFIND:
			r = "PROPFIND";
			break;

		case HTTP_VERB::HttpVerbPROPPATCH:
			r = "PROPPATCH";
			break;

		case HTTP_VERB::HttpVerbPUT:
			r = "PUT";
			break;

		case HTTP_VERB::HttpVerbSEARCH:
			r = "SEARCH";
			break;

		case HTTP_VERB::HttpVerbTRACE:
			r = "TRACE";
			break;

		case HTTP_VERB::HttpVerbTRACK:
			r = "TRACK";
			break;

		case HTTP_VERB::HttpVerbUNLOCK:
			r = "UNLOCK";
			break;

		case HTTP_VERB::HttpVerbUnknown: // FALL-THROUGH
			r = m_unknownVerb;
			break;

		case HTTP_VERB::HttpVerbInvalid:
		default:
			r = "INVALID";
			break;
		}

		return r;
	}

	QString
	req_t::getRawURL(void)
	{
		return m_rawUrl;
	}

	QString
	req_t::getAbsolutePath(void)
	{
		return m_path;
	}

	QString
	req_t::getURL(void)
	{
		return m_url;
	}

	QString
	req_t::getHost(void)
	{
		return m_host;
	}

	QString
	req_t::getQuery(void)
	{
		return m_query;
	}

	bool
	req_t::hasQueryString(void)
	{
		if (0 != m_query.length())
			return true;

		return false;
	}

	http::connection_id_t
	req_t::getConnectionId(void)
	{
		return m_connectId;
	}

	QString
	req_t::getVersionString(void)
	{
		QString ret = QString::number(m_version.MajorVersion) + "." + QString::number(m_version.MinorVersion);

		return ret;
	}

	unsigned short
	req_t::getVersionMajor(void)
	{
		return m_version.MajorVersion;
	}

	unsigned short
	req_t::getVersionMinor(void)
	{
		return m_version.MinorVersion;
	}

	std::size_t
	req_t::getTotalLength(void)
	{
		return m_totalLength;
	}

#define ATS_BSIZ 64
	QString
	req_t::addressToString(SOCKADDR& a)
	{
		QString				ret = "";
		const std::size_t	len = ATS_BSIZ;
		char				buf[ATS_BSIZ] = { 0 };
		PSOCKADDR_IN		sin = nullptr;
		PSOCKADDR_IN6		sin6 = nullptr;

		switch (a.sa_family) {
		case AF_INET:
			sin = reinterpret_cast<PSOCKADDR_IN>(&a);

			if (nullptr == ::InetNtopA(AF_INET, &sin->sin_addr, buf, len))
				throw std::runtime_error("http::req_t::getLocalAddress(): Error while calling ::InetNtop()");

			break;

		case AF_INET6:
			sin6 = reinterpret_cast<PSOCKADDR_IN6>(&a);

			if (nullptr == ::InetNtopA(AF_INET6, &sin6->sin6_family, buf, len))
				throw std::runtime_error("http::req_t::getLocalAddress(): Error while calling ::InetNtop()");

			break;

		default:
			throw std::runtime_error("http::req_t::getLocalAddress(): Unsupoorted address familt encountered");
			break;
		}

		return QString::fromUtf8(buf);
	}

	QString
	http::req_t::portToString(SOCKADDR& a)
	{
		QString			ret = "";
		PSOCKADDR_IN	sin = nullptr;
		PSOCKADDR_IN6	sin6 = nullptr;

		switch (a.sa_family) {
		case AF_INET:
			sin = reinterpret_cast<PSOCKADDR_IN>(&a);
			ret = QString::number(::ntohs(sin->sin_port));
			break;

		case AF_INET6:
			sin6 = reinterpret_cast<PSOCKADDR_IN6>(&a);
			ret = QString::number(::ntohs(sin6->sin6_port));
			break;

		default:
			throw std::runtime_error("http::req_t::portToString(): Unsupported address family encountered");
			break;
		}

		return ret;
	}

	QString
	http::req_t::toString(void)
	{
		QString ret("");

		ret += getVerbString() + " " + m_path + " HTTP/";
		ret += QString::number(m_version.MajorVersion) + "." + QString::number(m_version.MinorVersion) + " ";
		ret += "Local: " + m_laddr + ":" + m_lport + " Remote: " + m_raddr + ":" + m_rport + " ";
		ret += "RID: " + QString::number(m_id);

		return ret;
	}

};