#ifndef HAVE_HTTP_HANDLER_H
#define HAVE_HTTP_HANDLER_H

#include <QString>
#include <QList>

#include <cstdint>
#include <functional>
#include <memory>
#include <array>

#include "http_req.h"
#include "http_resp.h"

namespace http {

	typedef enum {	
				HTTP_HANDLER_POSTREAD = 0,	HTTP_HANDLERS_TRANSLATE_URI,	HTTP_HANDLERS_STORAGEMAP, 
				HTTP_HANDLERS_INIT,			HTTP_HANDLERS_HEADERPARSE,		HTTP_HANDLERS_ACCESS,
				HTTP_HANDLERS_AUTHEN,		HTTP_HANDLERS_AUTHZ,			HTTP_HANDLERS_CONTENT_TYPE,
				HTTP_HANDLERS_FIXUP,		HTTP_HANDLERS_RESPONSE,			HTTP_HANDLERS_LOG,
				HTTP_HANDLERS_CLEANUP,		HTTP_HANDLERS_IFILTER,			HTTP_HANDLERS_OFILTER,
				HTTP_HANDLERS_TYPE_COUNT
	} HTTP_HANDLERS_TYPE_T;

	typedef int8_t HTTP_HANDLER_RETURN_T;

	#define HTTP_HANDLER_RETURN_OKAY 0
	#define HTTP_HANDLER_RETURN_DECLINED -1

	#define HTTP_STATUS_CONTINUE 100
	#define HTTP_STATUS_SWITCHING_PROTOCOLS 101
	#define HTTP_STATUS_PROCESSING 102
	#define HTTP_STATUS_OKAY 200
	#define HTTP_STATUS_CREATED 201
	#define HTTP_STATUS_ACCEPTED 202
	#define HTTP_STATUS_NON_AUTHORITATIVE 203	
	#define HTTP_STATUS_NO_CONTENT 204
	#define HTTP_STATUS_RESET_CONTENT 205
	#define HTTP_STATUS_PARTIAL_CONTENT 206
	#define HTTP_STATUS_MULTI_STATUS 207
	#define HTTP_STATUS_ALREADY_REPORTED 208
	#define HTTP_STATUS_IM_USED 226
	#define HTTP_STATUS_MULTIPLE_CHOICES 300
	#define HTTP_STATUS_MOVED_PERMANENTLY 301
	#define HTTP_STATUS_HTTP_FOUND 302
	#define HTTP_STATUS_SEE_OTHER 303
	#define HTTP_STATUS_NOT_MODIFIED 304
	#define HTTP_STATUS_USE_PROXY 305
	#define HTTP_STATUS_SWITCH_PROXY 306
	#define HTTP_STATUS_TEMPORARY_REDIRECT 307
	#define HTTP_STATUS_PERMANENT_REDIRECT 308
	#define HTTP_STATUS_BAD_REQUEST 400
	#define HTTP_STATUS_UNAUTHORIZED 401
	#define HTTP_STATUS_PAYMENT_REQUIRED 402
	#define HTTP_STATUS_FORBIDDEN 403	
	#define HTTP_STATUS_NOT_FOUND 404
	#define HTTP_STATUS_METHOD_NOT_ALLOWED 405
	#define HTTP_STATUS_NOT_ACCEPTABLE 406
	#define HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED 407
	#define HTTP_STATUS_HTTP_REQUEST_TIMEOUT 408
	#define HTTP_STATUS_CONFLICT 409
	#define HTTP_STATUS_GONE 410
	#define HTTP_STATUS_LENGTH_REQUIRED 411
	#define HTTP_STATUS_PRECONDITION_FAILED 412
	#define HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE 413
	#define HTTP_STATUS_REQUEST_URI_TOO_LONG 414
	#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE 415
	#define HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE 416
	#define HTTP_STATUS_EXPECTATION_FAILED 417
	#define HTTP_STATUS_IM_A_TEAPOT 418
	#define HTTP_STATUS_AUTHENTICATION_TIMEOUT 419
	#define HTTP_STATUS_ENHANCE_YOUR_CALM 420
	#define HTTP_STATUS_UNPROCESSABLE_ENTITY 422
	#define HTTP_STATUS_LOCKED 423
	#define HTTP_STATUS_FAILED_DEPENDENCY 424
	#define HTTP_STATUS_UNORDEDED_COLLECTION 425
	#define HTTP_STATUS_UPGRADE_REQUIRED 426
	#define HTTP_STATUS_PRECONDITION_REQUIRED 428
	#define HTTP_STATUS_TOO_MANY_REQUESTS 429
	#define HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE 431
	#define HTTP_STATUS_LOGIN_TIMEOUT 440
	#define HTTP_STATUS_NO_RESPONSE 444
	#define HTTP_STATUS_BLOCKED_BY_WINDOWS_PARENTAL_CONTROLS 450
	#define HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS 451
	#define HTTP_STATUS_REQUEST_HEADER 494
	#define HTTP_STATUS_CERTIFICATE_ERROR 495
	#define HTTP_STATUS_HTTP_NO_CERTIFICATE 496
	#define HTTP_STATUS_HTTP_TO_HTTPS 497
	#define HTTP_STATUS_CLIENT_CLOSED_REQUEST 499
	#define HTTP_STATUS_INTERNAL_SERVER_ERROR 500
	#define HTTP_STATUS_NOT_IMPLEMENTED 501
	#define HTTP_STATUS_BAD_GATEWAY 502
	#define HTTP_STATUS_SERVICE_UNAVAILABLE 503
	#define HTTP_STATUS_GATEWAY_TIMEOUT 504
	#define HTTP_STATUS_VERSION_NOT_SUPPORTED 505
	#define HTTP_STATUS_VARIANT_ALSO_NEGOTIATES 506
	#define HTTP_STATUS_INSUFFICIENT_STORAGE 507
	#define HTTP_STATUS_LOOP_DETECTED 508
	#define HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED 509
	#define HTTP_STATUS_NOT_EXTENDED 510
	#define HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED 511
	#define HTTP_STATUS_ORIGIN_ERROR 520
	#define HTTP_STATUS_CONNECTION_TIMED_OUT 522
	#define HTTP_STATUS_PROXY_DECLINED_REQUEST 523
	#define HTTP_STATUS_A_TIMEOUT_OCCURRED 524	
	#define HTTP_STATUS_NETWORK_READ_TIMEOUT 598
	#define HTTP_STATUS_NETWORK_CONNECT_TIMEOUT 599

	class handler_t {
		private:
		protected:
			workerLog_t&	m_log;
			QString			m_name;

		public:
			handler_t(workerLog_t& l) : m_log(l) {}
			virtual ~handler_t(void) {}
			virtual QString getName(void) { return m_name; }
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_wptr_t&, http::resp_wptr_t&) = 0;
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_sptr_t&, http::resp_sptr_t&) = 0;
	};

	class error_handler_t {
		private:
		protected:
			workerLog_t&	m_log;
			QString			m_name;

		public:
			error_handler_t(workerLog_t& l) : m_log(l) {}
			virtual ~error_handler_t(void) { }
			virtual QString getName(void) { return m_name; }
			virtual bool operator()(http::status_t, http::req_wptr_t&, http::resp_wptr_t&) = 0;
			virtual bool operator()(http::status_t, http::req_sptr_t&, http::resp_sptr_t&) = 0;
	};

	//typedef std::unique_ptr< handler_t > handler_ptr_t;
	typedef QList < handler_t* > http_handler_list_t;
	typedef std::array< http_handler_list_t, HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_TYPE_COUNT >  http_handler_array_t;

	typedef QList< error_handler_t* > http_error_handler_list_t;

	// typedef std::function< HTTP_HANDLER_RETURN_T(http::req_wptr_t, http::resp_wptr_t) > http_handler_t;
	// typedef QList< http_handler_t > http_handler_list_t;
	// typedef std::array< http_handler_list_t, HTTP_HANDLERS_TYPE_T::HTTP_HANDLERS_TYPE_COUNT >  http_handler_array_t;
		
	typedef std::function< QString(void) > plugin_getName_t;
	typedef std::function< http_handler_array_t(void) > plugin_getHandlers_t;
	typedef std::function< bool(QString&, QSettings&) > plugin_initHandler_t;
		
	typedef std::shared_ptr< http_handler_array_t > handler_array_sptr_t;

};

#endif