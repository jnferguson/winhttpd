#include "http_hdr.h"

namespace http {
	
	std::array< hdr_t::hdr_entry_t, 31 >
		hdr_t::m_reqhdr = {
		HTTP_HEADER_ID::HttpHeaderCacheControl, "CACHE-CONTROL",
		HTTP_HEADER_ID::HttpHeaderConnection, "CONNECTION",
		HTTP_HEADER_ID::HttpHeaderDate, "DATE",
		HTTP_HEADER_ID::HttpHeaderKeepAlive, "KEEP-ALIVE",
		HTTP_HEADER_ID::HttpHeaderPragma, "PRAGMA",
		HTTP_HEADER_ID::HttpHeaderTrailer, "TRAILER",
		HTTP_HEADER_ID::HttpHeaderTransferEncoding, "TRANSFER-ENCODING",
		HTTP_HEADER_ID::HttpHeaderUpgrade, "UPGRADE",
		HTTP_HEADER_ID::HttpHeaderVia, "VIA",
		HTTP_HEADER_ID::HttpHeaderWarning, "WARNING",
		HTTP_HEADER_ID::HttpHeaderAllow, "ALLOW",
		HTTP_HEADER_ID::HttpHeaderContentLength, "CONTENT-LENGTH",
		HTTP_HEADER_ID::HttpHeaderContentType, "CONTENT-TYPE",
		HTTP_HEADER_ID::HttpHeaderContentEncoding, "CONTENT-ENCODING",
		HTTP_HEADER_ID::HttpHeaderContentLanguage, "CONTENT-LANGUAGE",
		HTTP_HEADER_ID::HttpHeaderContentLocation, "CONTENT-LOCATION",
		HTTP_HEADER_ID::HttpHeaderContentMd5, "CONTENT-MD5",
		HTTP_HEADER_ID::HttpHeaderContentRange, "CONTENT-RANGE",
		HTTP_HEADER_ID::HttpHeaderExpires, "EXPIRES",
		HTTP_HEADER_ID::HttpHeaderLastModified, "LAST-MODIFIED",
		HTTP_HEADER_ID::HttpHeaderAcceptRanges, "ACCEPT-RANGES",
		HTTP_HEADER_ID::HttpHeaderAge, "AGE",
		HTTP_HEADER_ID::HttpHeaderEtag, "ETAG",
		HTTP_HEADER_ID::HttpHeaderLocation, "LOCATION",
		HTTP_HEADER_ID::HttpHeaderProxyAuthenticate, "PROXY-AUTHENTICATE",
		HTTP_HEADER_ID::HttpHeaderRetryAfter, "RETRY-AFTER",
		HTTP_HEADER_ID::HttpHeaderServer, "SERVER",
		HTTP_HEADER_ID::HttpHeaderSetCookie, "SET-COOKIE",
		HTTP_HEADER_ID::HttpHeaderVary, "VARY",
		HTTP_HEADER_ID::HttpHeaderWwwAuthenticate, "WWW-AUTHENTICATE",
		HTTP_HEADER_ID::HttpHeaderRequestMaximum, ""
	};

	std::array< hdr_t::hdr_entry_t, 42 >
		hdr_t::m_resphdr = {
		HTTP_HEADER_ID::HttpHeaderCacheControl, "CACHE-CONTROL",
		HTTP_HEADER_ID::HttpHeaderConnection, "CONNECTION",
		HTTP_HEADER_ID::HttpHeaderDate, "DATE",
		HTTP_HEADER_ID::HttpHeaderKeepAlive, "KEEP-ALIVE",
		HTTP_HEADER_ID::HttpHeaderPragma, "PRAGMA",
		HTTP_HEADER_ID::HttpHeaderTrailer, "TRAILER",
		HTTP_HEADER_ID::HttpHeaderTransferEncoding, "TRANSFER-ENCODING",
		HTTP_HEADER_ID::HttpHeaderUpgrade, "UPGRADE",
		HTTP_HEADER_ID::HttpHeaderVia, "VIA",
		HTTP_HEADER_ID::HttpHeaderWarning, "WARNING",
		HTTP_HEADER_ID::HttpHeaderAllow, "ALLOW",
		HTTP_HEADER_ID::HttpHeaderContentLength, "CONTENT-LENGTH",
		HTTP_HEADER_ID::HttpHeaderContentType, "CONTENT-TYPE",
		HTTP_HEADER_ID::HttpHeaderContentEncoding, "CONTENT-ENCODING",
		HTTP_HEADER_ID::HttpHeaderContentLanguage, "CONTENT-LANGUAGE",
		HTTP_HEADER_ID::HttpHeaderContentLocation, "CONTENT-LOCATION",
		HTTP_HEADER_ID::HttpHeaderContentMd5, "CONTENT-MD5",
		HTTP_HEADER_ID::HttpHeaderContentRange, "CONTENT-RANGE",
		HTTP_HEADER_ID::HttpHeaderExpires, "EXPIRES",
		HTTP_HEADER_ID::HttpHeaderLastModified, "LAST-MODIFIED",
		HTTP_HEADER_ID::HttpHeaderAccept, "ACCEPT",
		HTTP_HEADER_ID::HttpHeaderAcceptCharset, "ACCEPT-CHARSET",
		HTTP_HEADER_ID::HttpHeaderAcceptEncoding, "ACCEPT-ENCODING",
		HTTP_HEADER_ID::HttpHeaderAcceptLanguage, "ACCEPT-LANGUAGE",
		HTTP_HEADER_ID::HttpHeaderAuthorization, "AUTHORIZATION",
		HTTP_HEADER_ID::HttpHeaderCookie, "COOKIE",
		HTTP_HEADER_ID::HttpHeaderExpect, "EXPECT",
		HTTP_HEADER_ID::HttpHeaderFrom, "FROM",
		HTTP_HEADER_ID::HttpHeaderHost, "HOST",
		HTTP_HEADER_ID::HttpHeaderIfMatch, "IF-MATCH",
		HTTP_HEADER_ID::HttpHeaderIfModifiedSince, "IF-MODIFIED-SINCE",
		HTTP_HEADER_ID::HttpHeaderIfNoneMatch, "IF-NONE-MATCH",
		HTTP_HEADER_ID::HttpHeaderIfRange, "IF-RANGE",
		HTTP_HEADER_ID::HttpHeaderIfUnmodifiedSince, "IF-UNMODIFIED-SINCE",
		HTTP_HEADER_ID::HttpHeaderMaxForwards, "MAX-FORWARDS",
		HTTP_HEADER_ID::HttpHeaderProxyAuthorization, "PROXY-AUTHORIZATION",
		HTTP_HEADER_ID::HttpHeaderReferer, "REFERER",
		HTTP_HEADER_ID::HttpHeaderRange, "RANGE",
		HTTP_HEADER_ID::HttpHeaderTe, "TE",
		HTTP_HEADER_ID::HttpHeaderTranslate, "TRANSLATE",
		HTTP_HEADER_ID::HttpHeaderUserAgent, "USER-AGENT",
		HTTP_HEADER_ID::HttpHeaderRequestMaximum, ""
	};

	hdr_t::hdr_t(workerLog_t& l) : m_log(l)
	{
		return;
	}

	hdr_t::~hdr_t(void)
	{
		return;
	}

	void
	hdr_t::addHeader(HTTP_HEADER_ID key, QString val, bool overwrite)
	{
		QString				ret = getHeaderById(key);
		hdr_t::vec_entry_t	ent = { 0 };

		if (!ret.length())
			throw std::runtime_error("http::hdr_t::addHeader(HTTP_HEADER_ID): invalid header id specified");

		ent.isKnown = true;
		ent.id = key;
		ent.key = ret;
		ent.value = val;

		addEntry(ent, overwrite);
		return;
	}

	void
	hdr_t::addEntry(hdr_t::vec_entry_t& v, bool overwrite)
	{
		//std::lock_guard< std::mutex > l(m_mutex);

		for (auto itr = m_hdrs.begin(); itr != m_hdrs.end(); itr++) {
			if (itr->id == v.id) {
				if (false == overwrite)
					return;

				itr->id = v.id;
				itr->isKnown = v.isKnown;
				itr->key = v.key;
				itr->value = v.value;
				return;
			}
		}

		m_hdrs.push_back(v);
		return;
	}

	void
	hdr_t::addHeader(QString key, QString val, bool overwrite)
	{
		HTTP_HEADER_ID		ret = getHeaderByName(key);
		hdr_t::vec_entry_t	ent = { 0 };

		if (HTTP_HEADER_ID::HttpHeaderRequestMaximum == ret)
			ent.isKnown = false;
		else
			ent.isKnown = true;

		ent.id = ret;
		ent.key = key;
		ent.value = val;

		addEntry(ent, overwrite);
		return;
	}

	void
	hdr_t::setHeader(HTTP_HEADER_ID key, QString val)
	{
		addHeader(key, val, true);
		return;
	}

	void
	hdr_t::setHeader(QString key, QString val)
	{
		addHeader(key, val, true);
		return;
	}

	HTTP_HEADER_ID
	hdr_t::getHeaderByName(QString& key)
	{
		for (auto itr = m_resphdr.begin(); itr != m_resphdr.end(); itr++)
			if (!key.compare(itr->name, Qt::CaseInsensitive))
				return itr->id;

		for (auto itr = m_reqhdr.begin(); itr != m_reqhdr.end(); itr++)
			if (!key.compare(itr->name, Qt::CaseInsensitive))
				return itr->id;

		return static_cast<HTTP_HEADER_ID>(HTTP_HEADER_ID::HttpHeaderRequestMaximum); // HttpHeaderRequestMaximum = HttpHeaderRequestMaximum = not a known header
	}

	QString
	hdr_t::getHeaderById(HTTP_HEADER_ID& key)
	{

		for (auto itr = m_resphdr.begin(); itr != m_resphdr.end(); itr++)
			if (key == itr->id)
				return itr->name;

		for (auto itr = m_reqhdr.begin(); itr != m_reqhdr.end(); itr++)
			if (key == itr->id)
				return itr->name;

		return ""; // "" = invalid
	}

	HTTP_REQUEST_HEADERS*
	hdr_t::getRequestHeaders(void)
	{
		//std::lock_guard< std::mutex >	l(m_mutex);
		HTTP_REQUEST_HEADERS*			r(nullptr);
		QVector< HTTP_UNKNOWN_HEADER* > vec;
		std::size_t						vsize(0);

		r = new HTTP_REQUEST_HEADERS;

		::memset(r, 0, sizeof(HTTP_REQUEST_HEADERS));

		r->UnknownHeaderCount = 0;
		r->TrailerCount = 0;
		r->pTrailers = nullptr;

		for (auto itr = m_hdrs.begin(); itr != m_hdrs.end(); itr++) {
			if (true == itr->isKnown) {
				HTTP_KNOWN_HEADER*	h = new HTTP_KNOWN_HEADER;
				char*				v = nullptr;
				USHORT				l = 0;

				if (itr->id >= HTTP_HEADER_ID::HttpHeaderRequestMaximum)
					throw std::runtime_error("http::hdr_t::getHeaders(): Invalid header ID encountered while processing known HTTP headers");

				if (USHRT_MAX <= itr->value.length())
					throw std::runtime_error("http::hdr_t::getHeaders(): Overly long HTTP header value encountered while processing known HTTP headers");

				l = itr->value.toUtf8().length();
				h->RawValueLength = l;

				if (0 == l)
					continue;

				v = new char[l + 1];

				::memset(v, 0, l + 1);
				::memcpy_s(v, l + 1, itr->value.toUtf8().constData(), l);

				h->RawValueLength = l;
				h->pRawValue = v;
				r->KnownHeaders[itr->id] = *h;
			}
			else {
				HTTP_UNKNOWN_HEADER*	u(new HTTP_UNKNOWN_HEADER);
				char*					n(nullptr);
				char*					v(nullptr);

				if (USHRT_MAX <= r->UnknownHeaderCount)
					throw std::runtime_error("http::hdr_t::getHeaders(): Too many unknown headers specified");

				r->UnknownHeaderCount++;

				if (USHRT_MAX <= itr->key.toUtf8().length() || USHRT_MAX <= itr->value.toUtf8().length())
					throw std::runtime_error("http::hdr_t::getHeaders(): Overly long unknown header length or value");

				u->NameLength = itr->key.toUtf8().length();
				u->RawValueLength = itr->value.toUtf8().length();
				n = new char[u->NameLength + 1];
				v = new char[u->RawValueLength + 1];

				if (0 == itr->key.toUtf8().length() || 0 == itr->value.toUtf8().length())
					continue;

				::memset(n, 0, u->NameLength + 1);
				::memset(v, 0, u->RawValueLength + 1);
				::memcpy_s(n, u->NameLength + 1, itr->key.toUtf8().constData(), u->NameLength);
				::memcpy_s(v, u->RawValueLength + 1, itr->value.toUtf8().constData(), u->RawValueLength);

				u->pName = n;
				u->pRawValue = v;

				vec.push_back(u);
			}
		}

		// XXX JF FIXME int overflow
		vsize = vec.size()*sizeof(HTTP_UNKNOWN_HEADER);

		r->pUnknownHeaders = new HTTP_UNKNOWN_HEADER[vsize];
		::memset(r->pUnknownHeaders, 0, vsize);

		for (auto cnt = 0; cnt < vec.size(); cnt++) {
			if (sizeof(HTTP_UNKNOWN_HEADER) > vsize)
				throw std::runtime_error("http::hdr_t::getHeaders(): invalid vector header size, aborting.");

			::memcpy_s(&r->pUnknownHeaders[cnt], vsize, vec.at(cnt), sizeof(HTTP_UNKNOWN_HEADER));
			vsize -= sizeof(HTTP_UNKNOWN_HEADER);
		}

		return r;
	}

	HTTP_RESPONSE_HEADERS*
	hdr_t::getResponseHeaders(void)
	{
		//std::lock_guard< std::mutex >	l(m_mutex);
		HTTP_RESPONSE_HEADERS*			r(nullptr);
		QVector< HTTP_UNKNOWN_HEADER* > vec;
		std::size_t						vsize(0);

		r = new HTTP_RESPONSE_HEADERS;

		::memset(r, 0, sizeof(HTTP_RESPONSE_HEADERS));

		r->UnknownHeaderCount = 0;
		r->TrailerCount = 0;
		r->pTrailers = nullptr;

		for (auto cnt = 0; cnt < HTTP_HEADER_ID::HttpHeaderResponseMaximum; cnt++) {
			r->KnownHeaders[cnt].pRawValue = nullptr;
			r->KnownHeaders[cnt].RawValueLength = 0;
		}

		for (auto itr = m_hdrs.begin(); itr != m_hdrs.end(); itr++) {
			if (true == itr->isKnown) {
				HTTP_KNOWN_HEADER*	h = new HTTP_KNOWN_HEADER;
				char*				v = nullptr;
				USHORT				l = 0;

				if (itr->id >= HTTP_HEADER_ID::HttpHeaderResponseMaximum)
					throw std::runtime_error("http::hdr_t::getHeaders(): Invalid header ID encountered while processing known HTTP headers");

				if (USHRT_MAX <= itr->value.length())
					throw std::runtime_error("http::hdr_t::getHeaders(): Overly long HTTP header value encountered while processing known HTTP headers");

				l = itr->value.toUtf8().length();
				h->RawValueLength = l;

				if (0 == l)
					continue;

				v = new char[l + 1];

				::memset(v, 0, l + 1);
				::memcpy_s(v, l + 1, itr->value.toUtf8().constData(), l);

				h->RawValueLength = l;
				h->pRawValue = v;
				r->KnownHeaders[itr->id] = *h;

			}
			else {
				HTTP_UNKNOWN_HEADER*	u(new HTTP_UNKNOWN_HEADER);
				char*					n(nullptr);
				char*					v(nullptr);

				if (USHRT_MAX <= r->UnknownHeaderCount)
					throw std::runtime_error("http::hdr_t::getHeaders(): Too many unknown headers specified");

				r->UnknownHeaderCount++;

				if (USHRT_MAX <= itr->key.toUtf8().length() || USHRT_MAX <= itr->value.toUtf8().length())
					throw std::runtime_error("http::hdr_t::getHeaders(): Overly long unknown header length or value");

				u->NameLength = itr->key.toUtf8().length();
				u->RawValueLength = itr->value.toUtf8().length();
				n = new char[u->NameLength + 1];
				v = new char[u->RawValueLength + 1];

				if (0 == itr->key.toUtf8().length() || 0 == itr->value.toUtf8().length())
					continue;

				::memset(n, 0, u->NameLength + 1);
				::memset(v, 0, u->RawValueLength + 1);
				::memcpy_s(n, u->NameLength + 1, itr->key.toUtf8().constData(), u->NameLength);
				::memcpy_s(v, u->RawValueLength + 1, itr->value.toUtf8().constData(), u->RawValueLength);

				u->pName = n;
				u->pRawValue = v;

				vec.push_back(u);
			}
		}

		// XXX JF int overflow check here pls
		vsize = vec.size()*sizeof(HTTP_UNKNOWN_HEADER);

		r->pUnknownHeaders = new HTTP_UNKNOWN_HEADER[vsize];
		::memset(r->pUnknownHeaders, 0, vsize);

		for (auto cnt = 0; cnt < vec.size(); cnt++) {
			HTTP_UNKNOWN_HEADER* tmp(vec.at(cnt));

			if (sizeof(HTTP_UNKNOWN_HEADER) > vsize)
				throw std::runtime_error("http::hdr_t::getHeaders(): invalid vector header size, aborting.");

			if (nullptr == tmp)
				throw std::runtime_error("http::hdr_t::getHeaders(): invalid pointer in unknown header vector.");

			if (INT_MAX <= tmp->NameLength)
				throw std::runtime_error("http::hdr_t::getHeaders(): invalid unknown header name length.");

			if (INT_MAX <= tmp->RawValueLength)
				throw std::runtime_error("http::hdr_t::getHeaders(): invalud unknown header value length.");

			r->pUnknownHeaders[cnt].pName = new CHAR[tmp->NameLength + 1];
			r->pUnknownHeaders[cnt].pRawValue = new CHAR[tmp->RawValueLength + 1];

			r->pUnknownHeaders[cnt].NameLength = tmp->NameLength;
			r->pUnknownHeaders[cnt].RawValueLength = tmp->RawValueLength;

			::memset((void*)r->pUnknownHeaders[cnt].pName, 0, tmp->NameLength + 1);
			::memset((void*)r->pUnknownHeaders[cnt].pRawValue, 0, tmp->RawValueLength + 1);

			::memcpy_s((void*)r->pUnknownHeaders[cnt].pName, tmp->NameLength + 1, tmp->pName, tmp->NameLength);
			::memcpy_s((void*)r->pUnknownHeaders[cnt].pRawValue, tmp->RawValueLength + 1, tmp->pRawValue, tmp->RawValueLength);

			vsize -= sizeof(HTTP_UNKNOWN_HEADER);
		}

		return r;
	}

	void
	hdr_t::destroyRequestHeaders(HTTP_REQUEST_HEADERS* h)
	{
		if (nullptr == h)
			return;

		for (auto cnt = 0; cnt < HTTP_HEADER_ID::HttpHeaderRequestMaximum; cnt++) {
			if (0 != h->KnownHeaders[cnt].RawValueLength && nullptr != h->KnownHeaders[cnt].pRawValue) {

				delete h->KnownHeaders[cnt].pRawValue;

				h->KnownHeaders[cnt].pRawValue = nullptr;
				h->KnownHeaders[cnt].RawValueLength = 0;
			}
		}

		if (nullptr == h->pUnknownHeaders || 0 == h->UnknownHeaderCount)
			return;

		for (auto cnt = 0; cnt < h->UnknownHeaderCount; cnt++) {
			if (0 != h->pUnknownHeaders[cnt].NameLength && nullptr != h->pUnknownHeaders[cnt].pName) {
				delete h->pUnknownHeaders[cnt].pName;

				h->pUnknownHeaders[cnt].pName = nullptr;
				h->pUnknownHeaders[cnt].NameLength = 0;
			}

			if (0 != h->pUnknownHeaders[cnt].RawValueLength && nullptr != h->pUnknownHeaders[cnt].pRawValue) {
				delete h->pUnknownHeaders[cnt].pRawValue;

				h->pUnknownHeaders[cnt].pRawValue = nullptr;
				h->pUnknownHeaders[cnt].RawValueLength = 0;
			}
		}

		delete h->pUnknownHeaders;
		h->pUnknownHeaders = nullptr;

		delete h;
		return;
	}

	void
	hdr_t::destroyResponseHeaders(HTTP_RESPONSE_HEADERS* h)
	{

		if (nullptr == h)
			return;

		for (auto cnt = 0; cnt < HTTP_HEADER_ID::HttpHeaderRequestMaximum; cnt++) {
			if (0 != h->KnownHeaders[cnt].RawValueLength && nullptr != h->KnownHeaders[cnt].pRawValue) {
				delete h->KnownHeaders[cnt].pRawValue;

				h->KnownHeaders[cnt].pRawValue = nullptr;
				h->KnownHeaders[cnt].RawValueLength = 0;
			}
		}

		if (nullptr == h->pUnknownHeaders || 0 == h->UnknownHeaderCount)
			return;

		for (auto cnt = 0; cnt < h->UnknownHeaderCount; cnt++) {
			if (0 != h->pUnknownHeaders[cnt].NameLength && nullptr != h->pUnknownHeaders[cnt].pName) {
				delete h->pUnknownHeaders[cnt].pName;

				h->pUnknownHeaders[cnt].pName = nullptr;
				h->pUnknownHeaders[cnt].NameLength = 0;
			}

			if (0 != h->pUnknownHeaders[cnt].RawValueLength && nullptr != h->pUnknownHeaders[cnt].pRawValue) {
				delete h->pUnknownHeaders[cnt].pRawValue;

				h->pUnknownHeaders[cnt].pRawValue = nullptr;
				h->pUnknownHeaders[cnt].RawValueLength = 0;
			}
		}

		delete h->pUnknownHeaders;
		h->pUnknownHeaders = nullptr;

		delete h;
		return;
	}

	void
	hdr_t::clearHeaders(void)
	{
		m_hdrs.clear();
		return;
	}

};