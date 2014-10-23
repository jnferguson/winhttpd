#include "http_resp.h"

namespace http {

	resp_t::resp_t(workerLog_t& log) : m_id(0), m_status(500), m_log(log), m_headers(log)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		m_cache.Policy = HTTP_CACHE_POLICY_TYPE::HttpCachePolicyNocache;
		m_cache.SecondsToLive = 0;

		return;
	}

	resp_t::~resp_t(void)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		m_headers.clearHeaders();
		m_body.clear();

		m_id = 0;
		m_status = 500;

		return;
	}

	void
	resp_t::setId(http::id_t id)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		m_id = id;
		return;
	}

	void
	resp_t::setStatus(http::status_t status)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		m_status = status;
		return;
	}

	void
	resp_t::setCache(http::cache_t c)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);

		m_cache.Policy = c.Policy;
		m_cache.SecondsToLive = c.SecondsToLive;
		return;
	}

	void
	resp_t::addHeader(QString& k, QString& v)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);

		m_headers.addHeader(k, v);
		return;
	}

	void
	resp_t::setHeader(QString& k, QString& v)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);

		m_headers.setHeader(k, v);
		return;
	}

	void
	resp_t::setBody(http::resp_vec_t& v)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);

		m_body.clear();
		m_body = v;
		return;
	}

	void
	resp_t::setBody(uint8_t* v, std::size_t l)
	{
		//	std::lock_guard< std::mutex >	lck(m_mutex);
		QByteArray						a(reinterpret_cast<const char*>(v), l);

		if (nullptr == v || 0 == l)
			return;

		m_body.clear();
		m_body = a;
		return;

	}

	void
	resp_t::setBody(QString& v)
	{
		//	std::lock_guard< std::mutex >	l(m_mutex);
		http::resp_vec_t			a(v.toUtf8());

		setBody(a);
		return;
	}

	QString
	resp_t::getStatusString(void)
	{
		QString							r("");

		switch (m_status) {
		case 100:
			r = "Continue";
			break;
		case 101:
			r = "Switching Protocols";
			break;
		case 102:
			r = "Processing";
			break;
		case 200:
			r = "OK";
			break;
		case 201:
			r = "Created";
			break;
		case 202:
			r = "Accepted";
			break;
		case 203:
			r = "Non-Authoritative Information";
			break;
		case 204:
			r = "No Content";
			break;
		case 205:
			r = "Reset Content";
			break;
		case 206:
			r = "Partial Content";
			break;
		case 207:
			r = "Multi-Status";
			break;
		case 208:
			r = "Already Reported";
			break;
		case 226:
			r = "IM Used";
			break;
		case 300:
			r = "Multiple Choices";
			break;
		case 301:
			r = "Moved Permanently";
			break;
		case 302:
			r = "Found";
			break;
		case 303:
			r = "See Other";
			break;
		case 304:
			r = "Not Modified";
			break;
		case 305:
			r = "Use Proxy";
			break;
		case 306:
			r = "Switch Proxy";
			break;
		case 307:
			r = "Temporary Redirect";
			break;
		case 308:
			r = "Permanent Redirect";
			break;
		case 400:
			r = "Bad Request";
			break;
		case 401:
			r = "Unauthorized";
			break;
		case 402:
			r = "Payment Required";
			break;
		case 403:
			r = "Forbidden";
			break;
		case 404:
			r = "Not Found";
			break;
		case 405:
			r = "Method Not Allowed";
			break;
		case 406:
			r = "Not Acceptable";
			break;
		case 407:
			r = "Proxy Authentication Required";
			break;
		case 408:
			r = "Request Timeout";
			break;
		case 409:
			r = "Conflict";
			break;
		case 410:
			r = "Gone";
			break;
		case 411:
			r = "Length Required";
			break;
		case 412:
			r = "Precondition Failed";
			break;
		case 413:
			r = "Request Entity Too Large";
			break;
		case 414:
			r = "Request-URI Too Long";
			break;
		case 415:
			r = "Unsupported Media Type";
			break;
		case 416:
			r = "Requested Range Not Satisfiable";
			break;
		case 417:
			r = "Expectation Failed";
			break;
		case 418:
			r = "I'm a teapot";
			break;
		case 419:
			r = "Authentication Timeout";
			break;
		case 420:
			r = "Enhance Your Calm";
			break;
		case 422:
			r = "Unprocessable Entity";
			break;
		case 423:
			r = "Locked";
			break;
		case 424:
			r = "Failed Dependency";
			break;
			/*case 424:
				r = "Method Failure";
				break;*/
		case 425:
			r = "Unordered Collection";
			break;
		case 426:
			r = "Upgrade Required";
			break;
		case 428:
			r = "Precondition Required";
			break;
		case 429:
			r = "Too Many Requests";
			break;
		case 431:
			r = "Request Header Fields Too Large";
			break;
		case 440:
			r = "Login Timeout";
			break;
		case 444:
			r = "No Response";
			break;
		case 450:
			r = "Blocked by Windows Parental Controls";
			break;
		case 451:
			r = "Unavailable For Legal Reasons";
			break;
			/*case 451:
				r = "Redirect";
				break;*/
		case 494:
			r = "Request Header";
			break;
		case 495:
			r = "Cert Error";
			break;
		case 496:
			r = "No Cert";
			break;
		case 497:
			r = "HTTP to HTTPS";
			break;
		case 499:
			r = "Client Closed Request";
			break;
		case 500:
			r = "Internal Server Error";
			break;
		case 501:
			r = "Not Implemented";
			break;
		case 502:
			r = "Bad Gateway";
			break;
		case 503:
			r = "Service Unavailable";
			break;
		case 504:
			r = "Gateway Timeout";
			break;
		case 505:
			r = "HTTP Version Not Supported";
			break;
		case 506:
			r = "Variant Also Negotiates";
			break;
		case 507:
			r = "Insufficient Storage";
			break;
		case 508:
			r = "Loop Detected";
			break;
		case 509:
			r = "Bandwidth Limit Exceeded";
			break;
		case 510:
			r = "Not Extended";
			break;
		case 511:
			r = "Network Authentication Required";
			break;
		case 520:
			r = "Origin Error";
			break;
		case 522:
			r = "Connection timed out";
			break;
		case 523:
			r = "Proxy Declined Request";
			break;
		case 524:
			r = "A timeout occurred";
			break;
		case 598:
			r = "Network read timeout error";
			break;
		case 599:
			r = "Network connect timeout error";
			break;
		default:
			r = "UNKNOWN";
			break;
		}

		return r;
	}

	HTTP_RESPONSE*
	resp_t::toHttpResponse(void)
	{
		//	std::lock_guard< std::mutex >	lck(m_mutex);
		HTTP_RESPONSE*					resp(new HTTP_RESPONSE);
		HTTP_RESPONSE_HEADERS*			hdrs(m_headers.getResponseHeaders());
		std::string						sc(getStatusString().toStdString());
		//QString							sc(getStatusString());
		BYTE*							buf(nullptr);
		HTTP_DATA_CHUNK*				chunk(new HTTP_DATA_CHUNK);
		std::size_t						len(m_body.size());

		::memset(resp, 0, sizeof(HTTP_RESPONSE));

		resp->StatusCode = m_status;
		resp->pReason = new CHAR[sc.length() + 1];
		::memset((void*)resp->pReason, 0, sc.length() + 1);
		::memcpy_s((void*)resp->pReason, sc.length() + 1, sc.c_str(), sc.length());

		resp->ReasonLength = ::strlen(resp->pReason);

		if (INT_MAX <= len)
			throw std::runtime_error("http::resp_t::toHttpResponse(): Overly large HTTP response encountered");

		buf = new BYTE[len + 1];

		::memset(buf, 0, len + 1);
		::memcpy_s(buf, len + 1, m_body.constData(), len);

		chunk->DataChunkType = HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromMemory;
		chunk->FromMemory.pBuffer = buf;
		chunk->FromMemory.BufferLength = len;

		resp->pEntityChunks = chunk;
		resp->EntityChunkCount = 1;

		if (nullptr != hdrs) {
			::memcpy_s(resp->Headers.KnownHeaders, sizeof(resp->Headers.KnownHeaders), hdrs->KnownHeaders, sizeof(hdrs->KnownHeaders));

			resp->Headers.pTrailers = hdrs->pTrailers;
			resp->Headers.TrailerCount = hdrs->TrailerCount;
			resp->Headers.UnknownHeaderCount = hdrs->UnknownHeaderCount;

			if (0 != hdrs->UnknownHeaderCount) {
				//USHORT l = hdrs->UnknownHeaderCount;

				//if (USHRT_MAX-1 < l)
				//	throw std::runtime_error("http::resp_t::toHttpResponse(): Unusually large volume of unknown HTTP headers, aborting response.");


				//resp->Headers.pUnknownHeaders = new HTTP_UNKNOWN_HEADER[l+1];

				/*for (auto idx = 0; idx < l; idx++) {
					resp->Headers.pUnknownHeaders[idx].NameLength		= hdrs->pUnknownHeaders[idx].NameLength;
					resp->Headers.pUnknownHeaders[idx].RawValueLength	= hdrs->pUnknownHeaders[idx].RawValueLength;
					resp->Headers.pUnknownHeaders[idx].pName			= hdrs->pUnknownHeaders[idx].pName;
					*/

				resp->Headers.pUnknownHeaders = hdrs->pUnknownHeaders;

				//::memcpy_s(resp->Headers.pUnknownHeaders, (l+1)*sizeof(HTTP_UNKNOWN_HEADER), hdrs->pUnknownHeaders, l*sizeof(HTTP_UNKNOWN_HEADER));
			}

			//m_headers.destroyResponseHeaders(hdrs);

		}

		//m_log.info("unknown headers count: " + QString::number(resp->Headers.UnknownHeaderCount) + " name: " + QString(resp->Headers.pUnknownHeaders[0].pName) + " value: " + QString(resp->Headers.pUnknownHeaders[0].pRawValue));
		return resp;
	}

	http::id_t
	resp_t::getId(void)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		return m_id;
	}

	http::status_t
	resp_t::getStatus(void)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		return m_status;
	}

	http::cache_t
	resp_t::getCache(void)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		return m_cache;
	}

	http::resp_vec_t
	resp_t::getBody(void)
	{
		//	std::lock_guard< std::mutex > l(m_mutex);
		return m_body;
	}

	http::hdr_t
	resp_t::getHeaders(void)
	{
		return m_headers;
	}
};