#include "handler.h"

namespace http {

	handler_mgr_t::handler_mgr_t(workerLog_t& l) : m_log(l), m_fileh(nullptr), m_mimeh(nullptr), m_tlogh(nullptr), m_errh(nullptr)
	{
		//std::lock_guard< std::mutex >	lock(m_mutex);
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

		if (s.value("Plugins/FileHandler/Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_fileh = new file_handler_t(l);

			if (false == registerHandler(HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_RESPONSE, m_fileh)) {
				m_log.error("handler_mgr_t::handler_mgr_t(): Error while registering FileHandler plugin", eventlog_cat_t::CAT_WORKER, false);
				throw std::runtime_error("Error in handler_mgr_t::handler_mgr_t() while calling registerHandler()");
				return;
			}
		}
		
		if (s.value("Plugins/MimeHandler/Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_mimeh = new mime_handler_t(l);

			if (false == registerHandler(HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_CONTENT_TYPE, m_mimeh)) {
				m_log.error("handler_mgr_t::handler_mgr_t(): Error while registering MimeHandler plugin", eventlog_cat_t::CAT_WORKER, false);
				throw std::runtime_error("Error in handler_mgr_t::handler_mgr_t() while calling registerHandler()");
				return;
			}
		}

		if (s.value("Plugins/FlatTextFileLogger/Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_tlogh = new tlog_handler_t(l);

			if (false == registerHandler(HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_LOG, m_tlogh)) {
				m_log.error("handler_mgr_t::handler_mgr_t(): Error while registering MimeHandler plugin", eventlog_cat_t::CAT_WORKER, false);
				throw std::runtime_error("Error in handler_mgr_t::handler_mgr_t() while calling registerHandler()");
				return;
			}
		}


		m_errh = new default_error_handler_t(l);

		if (false == registerHandler(m_errh)) {
			m_log.error("handler_mgr_t::handler_mgr_t(): Error while registering the default error page handler", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("handler_mgr_t::handler_mgr_t(): Error while registering the default error page handler");
			return;
		}

		return;
	}

	handler_mgr_t::~handler_mgr_t(void)
	{
		std::lock_guard< std::mutex > lock(m_mutex);

		if (nullptr != m_fileh) {
			delete m_fileh;
			m_fileh = nullptr;
		}

		if (nullptr != m_mimeh) {
			delete m_mimeh;
			m_mimeh = nullptr;
		}

		if (nullptr != m_tlogh) {
			delete m_tlogh;
			m_tlogh = nullptr;
		}

		return;
	}

	bool
	handler_mgr_t::registerHandler(error_handler_t* h)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		if (nullptr == h)
			return false;

		m_ehandlers.push_front(h);
		return true;
	}

	bool 
	handler_mgr_t::registerHandler(HTTP_HANDLERS_TYPE_T type, handler_t* handler) //http_handler_t& handler)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		if (m_handlers.size() <= static_cast< std::size_t >(type))
			return false;

		if (nullptr == handler)
			return false;

		m_handlers[type].push_front(handler);
		return true;
	}

	bool 
	handler_mgr_t::registerHandler(HTTP_HANDLERS_TYPE_T type, http_handler_list_t& handlers)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		if (m_handlers.size() <= static_cast< std::size_t >(type))
			return false;

		for (auto i : handlers)
			m_handlers[type].push_front(i);

		return true;
	}

	bool 
	handler_mgr_t::registerHandler(http_handler_array_t& h)
	{
		std::lock_guard< std::mutex > l(m_mutex);

		for (auto idx = static_cast< std::size_t >(HTTP_HANDLERS_TYPE_T::HTTP_HANDLER_POSTREAD); 
			idx < static_cast< std::size_t >(HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_TYPE_COUNT); idx++)
			for (auto itr = h[idx].begin(); itr != h[idx].end(); itr++)
				m_handlers[idx].push_front(*itr);

		return true;
	}
	
	bool 
	handler_mgr_t::executeHandlers(http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		std::lock_guard< std::mutex >	l(m_mutex);
		http::resp_sptr_t				s(nullptr);

		executeVoid(m_handlers[HTTP_HANDLERS_IFILTER], req, resp);

		for (auto idx = static_cast< std::size_t >(HTTP_HANDLERS_TYPE_T::HTTP_HANDLER_POSTREAD); 
			idx < static_cast< std::size_t >(HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_TYPE_COUNT); idx++) {

			switch (idx) {
				case HTTP_HANDLER_POSTREAD:	
				case HTTP_HANDLERS_INIT:
				case HTTP_HANDLERS_HEADERPARSE:
				case HTTP_HANDLERS_ACCESS:
				case HTTP_HANDLERS_FIXUP:
				case HTTP_HANDLERS_LOG:
				case HTTP_HANDLERS_CLEANUP:
					executeAll(m_handlers[idx], req, resp);
					break;

				case HTTP_HANDLERS_TRANSLATE_URI:
				case HTTP_HANDLERS_STORAGEMAP:
				case HTTP_HANDLERS_AUTHEN:
				case HTTP_HANDLERS_AUTHZ:			
				case HTTP_HANDLERS_CONTENT_TYPE:
				case HTTP_HANDLERS_RESPONSE:
					executeFirst(m_handlers[idx], req, resp);
					break;

				case HTTP_HANDLERS_IFILTER:
				case HTTP_HANDLERS_OFILTER:
					break;

				default:
					throw std::runtime_error("handler_t::executeHandlers(): Impossible run-state encountered (bug)");
					break;
			}
		}

		executeVoid(m_handlers[HTTP_HANDLERS_OFILTER], req, resp);

		s = resp.lock();

		if (nullptr == s.get()) {
			m_log.error("handler_mgr_t::executeHandlers(): Error while checking response for error; response object is null", eventlog_cat_t::CAT_WORKER, false);
			return false;
		}

		if (HTTP_STATUS_BAD_REQUEST <= s->getStatus()) 
			return executeErrorHandler(s->getStatus(), req, resp);

		s = nullptr;

		return true;
	}

	bool
	handler_mgr_t::executeErrorHandler(http::status_t c, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		bool							r(false);

		for (auto e : m_ehandlers) {
			r = e->operator()(c, req, resp);

			if (true == r)
				break;
		}

		return r;
	}

	void 
	handler_mgr_t::executeVoid(http_handler_list_t& v, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{

		for (auto e : v)
			e->operator()(req, resp);

		return;
	}
	
	bool
	handler_mgr_t::executeAll(http_handler_list_t& v, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		bool							okay = false;

		for (auto e : v)
			switch (e->operator()(req, resp)) {
				case HTTP_HANDLER_RETURN_DECLINED:
					break;
				case HTTP_HANDLER_RETURN_OKAY:
					okay = true;
					break;

				default:
					break;
			}

		return okay;
	}

	bool
	handler_mgr_t::executeFirst(http_handler_list_t& v, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		bool							okay = false;

		for (auto e : v)
			switch (e->operator()(req, resp)) {

				case HTTP_HANDLER_RETURN_DECLINED:
					break;

				case HTTP_HANDLER_RETURN_OKAY:
					return true;
					break;

				default:
					return okay;
					break;
			}


		return okay;
	}

	bool 
	handler_mgr_t::executeHandlers(http::status_t c, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		std::lock_guard< std::mutex >	l(m_mutex);

		return executeErrorHandler(c, req, resp);
	}

	bool
	handler_mgr_t::executeHandlers(HTTP_HANDLERS_TYPE_T type, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		std::lock_guard< std::mutex >	l(m_mutex);
		bool							r(false);
		http::resp_sptr_t				s(nullptr);

		if (m_handlers.size() <= static_cast< std::size_t >(type))
			return false;

		switch (type) {
			case HTTP_HANDLER_POSTREAD:	
			case HTTP_HANDLERS_INIT:
			case HTTP_HANDLERS_HEADERPARSE:
			case HTTP_HANDLERS_ACCESS:
			case HTTP_HANDLERS_FIXUP:
			case HTTP_HANDLERS_LOG:
			case HTTP_HANDLERS_CLEANUP:
				r = executeAll(m_handlers[type], req, resp);
				break;
			case HTTP_HANDLERS_TRANSLATE_URI:
			case HTTP_HANDLERS_STORAGEMAP:
			case HTTP_HANDLERS_AUTHEN:
			case HTTP_HANDLERS_AUTHZ:			
			case HTTP_HANDLERS_CONTENT_TYPE:
			case HTTP_HANDLERS_RESPONSE:
				r = executeFirst(m_handlers[type], req, resp);
				break;
			case HTTP_HANDLERS_IFILTER:
			case HTTP_HANDLERS_OFILTER:
				executeVoid(m_handlers[type], req, resp);
				r = true;
				break;
			default:
				// throw ??
				r = false;
				break;
		}

		s = resp.lock();

		if (nullptr == s.get()) {
			m_log.error("handler_mgr_t::executeHandlers(): Error while checking response for error; response object is null", eventlog_cat_t::CAT_WORKER, false);
			return false;
		}
	
		if (HTTP_STATUS_BAD_REQUEST <= s->getStatus()) 
			return executeErrorHandler(s->getStatus(), req, resp);
		
			

		return r;
	}

};