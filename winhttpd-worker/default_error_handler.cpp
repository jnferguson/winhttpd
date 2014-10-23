#include "default_error_handler.h"

namespace http {

	const QString default_error_handler_t::m_contact = "					If the problem persists or you have any questions or comments, please contact the service administrator at %1.";
	const QString default_error_handler_t::m_signature = "					WINHTTPD %1 Server at %2";
	const QString default_error_handler_t::m_template = 
														"<!DOCTYPE html>\r\n"
														"<html>\r\n"
														"	<head>\r\n"
														"		<meta charset=\"UTF-8\" />\r\n"
														"		<title>%1</title>\r\n"		
														"		<style>\r\n"
														"\r\n"
														"			html, body {\r\n"
														"				display: -moz-flex;\r\n"
														"				display: -ms-flex;\r\n"				
														"				display: -webkit-flex;\r\n"
														"				display: flex;\r\n"
														"				height: 90%;\r\n"
														"				width: 100%;\r\n"
														"			}\r\n"
														"\r\n"
														"			#container {\r\n"
														"				display: -moz-flex;\r\n"
														"				display: -ms-flex;\r\n"
														"				display: -webkit-flex;\r\n"
														"				display: flex;\r\n"
														"				margin: auto;\r\n"
														"				border: 5px solid darkblue;\r\n"
														"				border-radius: 15px;\r\n"
														"				box-shadow: 5px 5px 5px #888;\r\n"
														"				max-width: 60%;\r\n"
														"				padding: 1% 1% 1% 1%;\r\n"
														"			}\r\n"
														"\r\n"			
														"			#logo {\r\n"
														"				-moz-flex: initial;\r\n"
														"				-ms-flex: initial;\r\n"
														"				-webkit-flex: initial;\r\n"
														"				flex: initial;\r\n"
														"				width: 171px;\r\n"
														"				min-width: 171px;\r\n"
														"				height: 134px;\r\n"
														"				min-height: 134px;\r\n"
														"				background-image: url(\"/winhttpd-logo.png\");\r\n"
														"			}\r\n"
														"\r\n"			
														"			#content {\r\n"
														"				-moz-flex: 2;\r\n"
														"				-ms-flex: 2;\r\n"
														"				-webkit-flex: 2;\r\n"
														"				flex: 2;\r\n"
														"			}\r\n"
														"\r\n"
														"			h2 {\r\n"
														"				text-align: right;\r\n"
														"				text-shadow: 1px 1px 1px #888;\r\n"
														"			}\r\n"
														"\r\n"
														"			#hr {\r\n"
														"				background-color: darkblue;\r\n"
														"				border-radius: 15px;\r\n"
														"				height: 2px;\r\n"
														"				box-shadow: 1px 1px 1px #888;\r\n"
														"			}\r\n"
														"\r\n"
														"			p {\r\n"
														"				//text-shadow: 1px 1px 1px #888;\r\n"
														"			}\r\n"
														"		</style>\r\n"
														"	</head>\r\n"
														"	<body lang=\"en\">\r\n"
														"		<div id=\"container\">\r\n"
														"			<div id=\"logo\"></div>\r\n"
														"			<div id=\"content\">\r\n"
														"				<h2>%2</h2>\r\n"
														"				<div id=\"hr\"></div>\r\n"
														"				<p>%3</p>\r\n"
														"				%4\r\n"
														"				%5\r\n"
														"			</div>\r\n"
														"		</div>\r\n"
														"	</body>\r\n"
														"</html>\r\n";
	
	const std::array< default_error_handler_t::http_status_string_t, 51 >
	default_error_handler_t::m_vec = { {
				{ HTTP_STATUS_CONTINUE, "100 CONTINUE", "The server has received and accepted the request headers. Your browser should continue and send the request entity body at this time." },
				{ HTTP_STATUS_SWITCHING_PROTOCOLS, "101 SWITCHING PROTOCOLS", "Your browser has requested the server to switch protocols and the server is complying." },
				{ HTTP_STATUS_PROCESSING, "102 PROCESSING", "The server has received and is processing the request, but no response is available yet." },
				{ HTTP_STATUS_CREATED, "201 CREATED", "The request has been fulfilled and resulted in a new resource being created." },
				{ HTTP_STATUS_ACCEPTED, "202 ACCEPTED", "The request has been accepted for processing, but the processing has not been completed. The request might or might not eventually be acted upon, as it might be disallowed when processing actually takes place." },
				{ HTTP_STATUS_NON_AUTHORITATIVE, "203 NON-AUTHORITATIVE", "The server successfully processed the request, but is returning information that may be from another source." },
				{ HTTP_STATUS_NO_CONTENT, "204 NO CONTENT", "The server successfully processed the request, but is not returning any content." },
				{ HTTP_STATUS_RESET_CONTENT, "205 RESET CONTENT", "The server successfully processed the request, but is not returning any content. Your browser needs to reset its local content at this time." },
				{ HTTP_STATUS_NOT_MODIFIED, "304 NOT MODIFIED", " The requested resource has not been modified since the version specified by the request headers If-Modified-Since or If-Match. This means that there is no need to retransmit the resource, since the client still has a previously-downloaded copy." },
				{ HTTP_STATUS_BAD_REQUEST, "400 BAD REQUEST", "Your browser sent a request that this server could not understand." },
				{ HTTP_STATUS_UNAUTHORIZED, "401 AUTHORIZATION REQUIRED", "The server could not verify that you are authorized to access the document you requested. Either you supplied the wrong credentials (e.g. bad password), or your browser doesn't understand how to supply the credentials required." },
				{ HTTP_STATUS_PAYMENT_REQUIRED, "402 PAYMENT REQUIRED", "The resource you are attempting to access requires payment." },
				{ HTTP_STATUS_FORBIDDEN, "403 FORBIDDEN", "You do not have permission to access %1 on this server. Please check the URL to ensure its validity." },
				{ HTTP_STATUS_NOT_FOUND, "404 NOT FOUND", "The requested URL %1 was not found on this server." },
				{ HTTP_STATUS_METHOD_NOT_ALLOWED, "405 METHOD NOT ALLOWED", "The request method %1 is inapropriate for the URL %2." },
				{ HTTP_STATUS_NOT_ACCEPTABLE, "406 NOT ACCEPTABLE", "An appropriate representation of the resource %1 could not be found on this server." },
				{ HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED, "407 PROXY AUTHENTICATION REQUIRED", "You must authenticate with a proxy server before this request can be serviced. Please logon to your proxy server and then try again." },
				{ HTTP_STATUS_HTTP_REQUEST_TIMEOUT, "408 HTTP REQUEST TIMEOUT", "Your browser did not produce a request within the time that the server was prepared to wait. You may repeat the request without modifications at any later time." },
				{ HTTP_STATUS_CONFLICT, "409 CONFLICT", "The request could not be completed due to a conflict with the current state of the resource." },
				{ HTTP_STATUS_GONE, "410 GONE", "The requested resource, %1, is no longer available on this server and there is no forwarding address. Please remove all references to this resource." },
				{ HTTP_STATUS_LENGTH_REQUIRED, "411 LENGTH REQUIRED", "Your browser failed to specify a length of the content it was submitting to the server, which is required." },
				{ HTTP_STATUS_PRECONDITION_FAILED, "412 PRECONDITION FAILED", "The request was not completed due to preconditions that are set by your browser in the request header. Preconditions prevent the requested method from being applied to a resource other than the one intended. An example of a precondition is testing for expired content in the page cache of your browser." },
				{ HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE, "413 REQUEST ENTITY TOO LARGE", "The requested resource %1 does not allow request data with POST requests, or the amount of data provided in the request exceeds the capacity limit." },
				{ HTTP_STATUS_REQUEST_URI_TOO_LONG, "414 REQUEST URI TOO LONG", "The requested URLs length exceeds the capacity limit for this server" },
				{ HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, "415 UNSUPPORTED MEDIA TYPE", "The server refused this request because the request entity is in a format not supported by the requested resource for the requested method." },
				{ HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE, "416 REQUESTED RANGE NOT SATISFIABLE", "None of the range-specifier values in the Range request header field overlap the current extent of the selected resource." },
				{ HTTP_STATUS_EXPECTATION_FAILED, "417 EXPECTATION FAILED", "The expectation given in the Expect request header field could not be met by this server." },
				{ HTTP_STATUS_AUTHENTICATION_TIMEOUT, "419 AUTHENTICATION TIMEOUT", "The previously valid authenticated session is not longer valid. This is likely due to a timeout and you need to log back in." },
				{ HTTP_STATUS_UNPROCESSABLE_ENTITY, "422 UNPROCESSABLE ENTITY", "The request was well-formed but was unable to be followed due to semantic errors." },
				{ HTTP_STATUS_LOCKED, "423 LOCKED", "The resource that is being accessed is locked." },
				{ HTTP_STATUS_FAILED_DEPENDENCY, "424 FAILED DEPENDENCY", "The request failed due to failure of a previous request." },
				{ HTTP_STATUS_UNORDEDED_COLLECTION, "425 UNORDEDED COLLECTION", "Your browser attempted to set the position of an internal collection member in an unordered collection or in a collection with a server-maintained ordering." },
				{ HTTP_STATUS_UPGRADE_REQUIRED, "426 UPGRADE REQUIRED", "Your browser should switch to a different protocol such as TLS/1.0." },
				{ HTTP_STATUS_PRECONDITION_REQUIRED, "428 PRECONDITION REQUIRED", "The origin server requires the request to be conditional. Intended to prevent the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict." },
				{ HTTP_STATUS_TOO_MANY_REQUESTS, "429 TOO MANY REQUESTS", "Your browser has exceeded the number of allowed requests in a given period of time. Please try again later." },
				{ HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE, "431 REQUEST HEADER FIELDS TOO LARGE", "The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large." },
				{ HTTP_STATUS_LOGIN_TIMEOUT, "440 LOGIN TIMEOUT", "Your login timed out or otherwise expired. Please login again to continue your session." },
				{ HTTP_STATUS_NO_RESPONSE, "444 NO RESPONSE", "There is no response to your request by the server. Furthermore, it has closed your connection." },
				{ HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS, "451 UNAVAILABLE FOR LEGAL REASONS", "The resource requested is unavailable for legal reasons. This could be the result of a civil suit, censorship, mandated blocked-access, self-removal due to national security letter or similar related legal matters. " },
				{ HTTP_STATUS_INTERNAL_SERVER_ERROR, "500 INTERNAL SERVER ERROR", "The server encountered an internal error or misconfiguration and was unable to complete your request." },
				{ HTTP_STATUS_NOT_IMPLEMENTED, "501 NOT IMPLEMENTED", "The requested resource cannot be displayed because a header value in the request does not match certain configuration settings on the server. For example, a request might specify a HTTP method of POST to a static file that cannot be posted to, or specify a Transfer-Encoding value that cannot make use of compression or similar." },
				{ HTTP_STATUS_BAD_GATEWAY, "502 BAD GATEWAY", "The server or proxy server returned an invalid or incomplete response." },
				{ HTTP_STATUS_SERVICE_UNAVAILABLE, "503 SERVICE UNAVAILABLE", "The requested service is unavailable." },
				{ HTTP_STATUS_GATEWAY_TIMEOUT, "504 GATEWAY TIMEOUT", "The server, while acting as a gateway or proxy, did not receive a timely response from the upstream server it accessed while attempting to complete this request." },
				{ HTTP_STATUS_VERSION_NOT_SUPPORTED, "505 HTTP VERSION NOT SUPPORTED", "The requested HTTP version %1 is not supported by this server." },
				{ HTTP_STATUS_VARIANT_ALSO_NEGOTIATES, "506 VARIANT ALSO NEGOTIATES", "Transparent content negotiation for the request results in a circular reference." },
				{ HTTP_STATUS_INSUFFICIENT_STORAGE, "507 INSUFFICIENT STORAGE", "The server is unable to store the representation needed to complete the request." },
				{ HTTP_STATUS_LOOP_DETECTED, "508 LOOP DETECTED", "The server detected an infinite loop while processing the request." },
				{ HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED, "509 BANDWIDTH LIMIT EXCEEDED", "The server is unable to service your request due to the site owner reaching his/her bandwidth limit. Please try again later." },
				{ HTTP_STATUS_NOT_EXTENDED, "510 NOT EXTENDED", "Further extensions to the request are required for the server to fulfill it." },
				{ HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED, "511 NETWORK AUTHENTICATION REQUIRED", "Your browser needs to authenticate to gain network access." }
	}};

	default_error_handler_t::default_error_handler_t(workerLog_t& l) : error_handler_t(l)
	{
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
		QString		contact(m_contact); // http::default_contact_segment);
		QString		signature(m_signature); //http::default_service_signature);
		QString		page(m_template); //http::default_error_page);
		QString		version("");
		QString		fqdn("");
		QString		admin("");

		s.beginGroup("General");

		version = s.value("Version", WINHTTPD_VERSION).toString();
		fqdn	= s.value("ServerName", "localhost").toString();
		admin	= s.value("ServerAdmin", QString("root@" + fqdn)).toString();

		if (! s.value("ServerSignature", "On").toString().compare("On", Qt::CaseInsensitive)) {
			signature = signature.arg(version, fqdn).toHtmlEscaped();
		}
		else
			signature = "";

		contact = contact.arg(admin).toHtmlEscaped();

		

		for (auto itr = m_vec.begin(); itr != m_vec.end(); itr++) 
			m_files.insert(itr->code, page.arg(itr->title, itr->title, itr->msg, QString("<p>"+contact+"</p>"), QString("<p>"+signature+"</p>")));
		
		/*for (std::size_t idx = 0; idx < STATUS_STRING_COUNT; idx++) {
			http::http_status_string_t* tmp = &http::status_strings[idx];

			m_files.insert(tmp->code, page.arg(tmp->title, tmp->title, tmp->msg, contact, signature));
		}*/
			
		m_log.info("default_error_handler_t: plugin initialized");
	}

	default_error_handler_t::~default_error_handler_t(void)
	{
		m_files.clear();
		return;
	}

	bool
	default_error_handler_t::operator()(http::status_t c, http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		http::resp_sptr_t	respPtr = resp.lock();
		http::req_sptr_t	reqPtr = req.lock();

		if (respPtr && reqPtr)
			return operator()(c, reqPtr, respPtr);


		return false;
	}

	bool
	default_error_handler_t::operator()(http::status_t c, http::req_sptr_t& req, http::resp_sptr_t& resp)
	{
		QMap< http::status_t, QString >::iterator	itr(nullptr);
		QString										ret("");


		itr = m_files.find(c);

		if (m_files.end() == itr) {
			m_log.error("default_error_handler_t::operator(): Failed to locate an error handler for HTTP status code: " + QString::number(c), eventlog_cat_t::CAT_WORKER, false);
			return false;
		}

		ret = itr.value();

		switch (itr.key()) {
			case HTTP_STATUS_VERSION_NOT_SUPPORTED:
				ret = ret.arg(req->getVersionString().toHtmlEscaped());
				break;
			case HTTP_STATUS_METHOD_NOT_ALLOWED:
				ret = ret.arg(req->getVerbString().toHtmlEscaped(), req->getAbsolutePath().toHtmlEscaped());
				break;
			case HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE:
			case HTTP_STATUS_NOT_ACCEPTABLE:
			case HTTP_STATUS_GONE:
			case HTTP_STATUS_NOT_FOUND:
			case HTTP_STATUS_FORBIDDEN:
				ret = ret.arg(req->getAbsolutePath().toHtmlEscaped());
				break;
			default:
				break;
		}

		resp->setStatus(c);
		resp->setBody(ret);
		resp->setHeader(QString("Content-Type"), QString("text/html"));
		return true;
	}
};