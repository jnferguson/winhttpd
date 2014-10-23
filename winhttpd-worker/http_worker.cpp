#include "http_worker.h"

#pragma comment(lib, "httpapi.lib")

namespace http {
	worker_t::worker_t(workerLog_t& l, QObject* p) : QObject(p), m_log(l), m_rqueue(nullptr), m_stop(false), m_logConnections(false)
	{
		std::lock_guard< std::mutex >	g(m_mutex[OBJECT_MUTEX]);
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

		s.beginGroup("Worker");
		m_logConnections = s.value("LogConnectionsInStatusLog", false).toBool();
		s.endGroup();

		m_threads[RESPONSE_THREAD] = nullptr;
		m_threads[REQUEST_THREAD] = nullptr;

		return;
	}

	worker_t::~worker_t(void)
	{
		std::lock_guard< std::mutex >	g(m_mutex[OBJECT_MUTEX]);
		std::lock_guard< std::mutex >	c(m_mutex[REQUEST_MUTEX]);
		std::lock_guard< std::mutex >	r(m_mutex[RESPONSE_MUTEX]);

		if (false == m_stop) {
			m_stop = true;
			QThread::msleep(1000);// ?? necessary -- find a better way ??
		}

		if (nullptr != m_rqueue) {
			::HttpCloseRequestQueue(m_rqueue);
			m_rqueue = nullptr;
		}


		if (nullptr != m_threads[RESPONSE_THREAD]) {
			delete m_threads[RESPONSE_THREAD];
			m_threads[RESPONSE_THREAD] = nullptr;
		}

		if (nullptr != m_threads[REQUEST_THREAD]) {
			delete m_threads[REQUEST_THREAD];
			m_threads[REQUEST_THREAD] = nullptr;
		}

		return;
	}

	void
	worker_t::setStop(bool s)
	{
		std::lock_guard< std::mutex > g(m_mutex[OBJECT_MUTEX]);

		m_stop = s;
		return;
	}

	bool
	worker_t::getStop(void)
	{
		std::lock_guard< std::mutex > g(m_mutex[OBJECT_MUTEX]);

		return m_stop;
	}

	bool
	worker_t::start(void)
	{
		std::lock_guard< std::mutex >	g(m_mutex[OBJECT_MUTEX]);
		std::lock_guard< std::mutex >	c(m_mutex[REQUEST_MUTEX]);
		std::lock_guard< std::mutex >	s(m_mutex[RESPONSE_MUTEX]);
		const HTTPAPI_VERSION			v = HTTPAPI_VERSION_2;
		ULONG							r = NO_ERROR;


		r = ::HttpInitialize(v, HTTP_INITIALIZE_SERVER, nullptr);

		if (NO_ERROR != r) {
			m_log.error("http::worker_t::start(): Error while calling ::HttpInitialize()");
			throw ::GetLastError();
		}

		r = ::HttpCreateRequestQueue(v, L"winhttpd", nullptr, HTTP_CREATE_REQUEST_QUEUE_FLAG_OPEN_EXISTING, &m_rqueue);

		if (NO_ERROR != r) {
			m_log.error("http::worker_t::start(): Error while calling ::HttpCreateRequestQueue()");
			throw ::GetLastError();
		}

		m_stop = false;

		if (nullptr != m_threads[REQUEST_THREAD])
			delete m_threads[REQUEST_THREAD];

		if (nullptr != m_threads[RESPONSE_THREAD])
			delete m_threads[RESPONSE_THREAD];

		m_threads[REQUEST_THREAD] = new std::thread(&worker_t::pollRequests, this);
		m_threads[RESPONSE_THREAD] = new std::thread(&worker_t::pollResponses, this);


		m_threads[REQUEST_THREAD]->detach();
		m_threads[RESPONSE_THREAD]->detach();

		return true;
	}

	void
	worker_t::stop(void)
	{
		std::lock_guard< std::mutex >	g(m_mutex[OBJECT_MUTEX]);
		std::lock_guard< std::mutex >	c(m_mutex[REQUEST_MUTEX]);
		std::lock_guard< std::mutex >	r(m_mutex[RESPONSE_MUTEX]);

		m_request_queue.clear();
		m_response_queue.clear();

		if (false == m_stop) {
			m_stop = true;
			QThread::msleep(1000); // ?? necessary -- find a better way ??
		}

		if (nullptr != m_rqueue) {
			::HttpCloseRequestQueue(m_rqueue);
			m_rqueue = nullptr;
		}

		if (nullptr != m_threads[RESPONSE_THREAD]) {
			delete m_threads[RESPONSE_THREAD];
			m_threads[RESPONSE_THREAD] = nullptr;
		}

		if (nullptr != m_threads[REQUEST_THREAD]) {
			delete m_threads[REQUEST_THREAD];
			m_threads[REQUEST_THREAD] = nullptr;
		}

		return;
	}

	void
	worker_t::pollRequests(void)
	{

		do {
			http::req_sptr_t req = this->getRequest();

			if (nullptr == req) {
				if (ERROR_OPERATION_ABORTED == ::GetLastError() && true == getStop())
					return;

				m_log.info("http::worker_t::pollRequests(): got nullptr back from getRequest()...");
				continue;
			}

			if (true == m_logConnections)
				logHttpRequest(req);

			putRequestOnInputQueue(req);
			req = nullptr;
		} while (false == this->getStop());

		return;
	}

	void
	worker_t::pollResponses(void)
	{
		do {
			http::resp_sptr_t r = getResponseFromOutputQueue();

			if (nullptr == r) {
				QThread::msleep(250);
				continue;
			}

			if (false == this->putResponse(r)) {
				m_log.error("http::worker_t::pollResponses(): Error while calling low_level_http_t::putResponse()", eventlog_cat_t::CAT_WORKER, false);
				r = nullptr;
				continue;
			}

			r = nullptr;
		} while (false == this->getStop());

		return;
	}

	http::req_sptr_t
	worker_t::getRequest(void)
	{
		http::req_sptr_t				rptr(nullptr);
		HTTP_REQUEST*					req(nullptr);
		BYTE*							buf(nullptr);
		ULONG							ret(NO_ERROR);
		ULONG							blen(1024 * 16);

		buf = new BYTE[blen];
		req = reinterpret_cast<HTTP_REQUEST*>(buf);

		::memset(buf, 0, blen);

		ret = ::HttpReceiveHttpRequest(m_rqueue, 0, HTTP_RECEIVE_REQUEST_FLAG_FLUSH_BODY, req, blen, nullptr, nullptr);

		if (NO_ERROR != ret) {
			::SetLastError(ret);

			if (ERROR_OPERATION_ABORTED == ret) {
				delete buf;
				return nullptr;
			}


			m_log.error("http::worker_t::getRequest(): Error while calling ::HttpReceiveHttpRequest()");
			delete buf;
			return nullptr;
		}

		rptr = std::shared_ptr< http::req_t >(new http::req_t(req, m_log));
		delete buf;
		return rptr;
	}

	bool
	worker_t::inputQueueIsEmpty(void)
	{
		std::lock_guard< std::mutex > l(m_mutex[REQUEST_MUTEX]);


		if (true == m_request_queue.isEmpty())
			return true;

		return false;
	}

	http::req_sptr_t
	worker_t::getRequestFromInputQueue(void)
	{
		std::lock_guard< std::mutex > l(m_mutex[REQUEST_MUTEX]);

		if (true == m_request_queue.isEmpty())
			return nullptr;

		return m_request_queue.takeLast();
	}

	bool
	worker_t::outputQueueIsEmpty(void)
	{
		std::lock_guard< std::mutex > l(m_mutex[RESPONSE_MUTEX]);

		if (true == m_response_queue.isEmpty())
			return true;

		return false;
	}

	http::resp_sptr_t
	worker_t::getResponseFromOutputQueue(void)
	{
		std::lock_guard< std::mutex > l(m_mutex[RESPONSE_MUTEX]);

		if (true == m_response_queue.isEmpty())
			return nullptr;

		return m_response_queue.takeLast();
	}

	void
	worker_t::putRequestOnInputQueue(http::req_sptr_t r)
	{
		std::lock_guard< std::mutex > l(m_mutex[REQUEST_MUTEX]);


		if (nullptr == r)
			return;

		m_request_queue.push_back(r);
		return;
	}

	void
	worker_t::putResponseOnOutputQueue(http::resp_sptr_t r)
	{
		std::lock_guard< std::mutex > l(m_mutex[RESPONSE_MUTEX]);

		if (nullptr == r)
			return;

		m_response_queue.push_back(r);
		return;
	}

	bool
	worker_t::putResponse(http::resp_sptr_t r)
	{
		HTTP_RESPONSE*						resp = nullptr;
		ULONG								len = 0;
		ULONG								ret = 0;

		if (nullptr == r) {
			m_log.error("http::worker_t::putResponse(): Invalid parameter specified to method (nullptr)", eventlog_cat_t::CAT_WORKER, false);
			return false;
		}

		resp = r->toHttpResponse();

		if (nullptr == resp) {
			m_log.error("http::worker_t::putResponse(): Invalid HTTP_REQUEST specified to method (nullptr)", eventlog_cat_t::CAT_WORKER, false);
			return false;
		}

		ret = ::HttpSendHttpResponse(m_rqueue,
			r->getId(),
			0,
			resp,
			&r->getCache(),
			&len,
			nullptr,
			0,
			nullptr,
			nullptr
			);

		if (NO_ERROR != ret) {
			::SetLastError(ret);
			m_log.error("http::worker_t::putResponse(): Error while calling ::HttpSendHttpResponse() returned " + QString::number(ret) + " ");
			::HttpCancelHttpRequest(m_rqueue, r->getId(), nullptr);
			return false;
		}

		if (0 != resp->ReasonLength && nullptr != resp->pReason) {
			delete resp->pReason;

			resp->pReason = nullptr;
			resp->ReasonLength = 0;
		}

		if (0 != resp->EntityChunkCount) {
			for (std::size_t cnt = 0; cnt < resp->EntityChunkCount; cnt++) {
				if (0 != resp->pEntityChunks[cnt].FromMemory.BufferLength && nullptr != resp->pEntityChunks[cnt].FromMemory.pBuffer) {
					delete resp->pEntityChunks[cnt].FromMemory.pBuffer;
					resp->pEntityChunks[cnt].FromMemory.pBuffer = nullptr;
					resp->pEntityChunks[cnt].FromMemory.BufferLength = 0;
				}
			}

			delete resp->pEntityChunks;
			resp->pEntityChunks = nullptr;
		}

		for (auto cnt = 0; cnt < HTTP_HEADER_ID::HttpHeaderResponseMaximum; cnt++) {
			if (0 != resp->Headers.KnownHeaders[cnt].RawValueLength && nullptr != resp->Headers.KnownHeaders[cnt].pRawValue) {
				delete resp->Headers.KnownHeaders[cnt].pRawValue;
				resp->Headers.KnownHeaders[cnt].pRawValue = nullptr;
				resp->Headers.KnownHeaders[cnt].RawValueLength = 0;
			}
		}

		if (0 != resp->Headers.UnknownHeaderCount || nullptr != resp->Headers.pUnknownHeaders) {
			for (auto cnt = 0; cnt < resp->Headers.UnknownHeaderCount; cnt++) {
				if (0 != resp->Headers.pUnknownHeaders[cnt].NameLength && nullptr != resp->Headers.pUnknownHeaders[cnt].pName) {
					delete resp->Headers.pUnknownHeaders[cnt].pName;

					resp->Headers.pUnknownHeaders[cnt].pName = nullptr;
					resp->Headers.pUnknownHeaders[cnt].NameLength = 0;
				}

				if (0 != resp->Headers.pUnknownHeaders[cnt].RawValueLength && nullptr != resp->Headers.pUnknownHeaders[cnt].pRawValue) {
					delete resp->Headers.pUnknownHeaders[cnt].pRawValue;

					resp->Headers.pUnknownHeaders[cnt].pRawValue = nullptr;
					resp->Headers.pUnknownHeaders[cnt].RawValueLength = 0;
				}
			}
		}

		delete resp->Headers.pUnknownHeaders;
		resp->Headers.pUnknownHeaders = nullptr;

		delete resp;
		resp = nullptr;

		r = nullptr;
		return true;
	}

	void
	worker_t::waitForClientDisconnect(http::req_sptr_t r)
	{
		ULONG ret = NO_ERROR;
		if (nullptr == r)
			return;

		::HttpWaitForDisconnect(m_rqueue, r->getId(), nullptr);
		return;
	}

	void
	worker_t::logHttpRequest(http::req_sptr_t r)
	{
		QString str("");

		if (nullptr == r)
			throw std::runtime_error("http::worker_t::logHttpRequest(): Invalid parameter passed (nullptr)");

		str += "RID: 0x" + QString::number(r->getId()).toUpper() + " CLIENT: " + r->getRemoteAddress() + ": ";
		str += "REQUEST: \"" + r->getVerbString() + " " + r->getAbsolutePath() + " HTTP/" + r->getVersionString() + "\"";

		m_log.info(str);
		return;
	}

};
