#ifndef HAVE_HANDLER_H
#define HAVE_HANDLER_H

#include <QString>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <mutex>

#include "workerlog.h"
#include "http_handler.h"
#include "file_handler.h"
#include "mime_handler.h"
#include "tlog_handler.h"
#include "default_error_handler.h"

namespace http {

	class handler_mgr_t
	{
		private:
			std::mutex					m_mutex;
			http::http_handler_array_t	m_handlers;
			http_error_handler_list_t	m_ehandlers;
			workerLog_t&				m_log;
			file_handler_t*				m_fileh;
			mime_handler_t*				m_mimeh;
			tlog_handler_t*				m_tlogh;
			default_error_handler_t*	m_errh;

		protected:

			virtual void executeVoid(http_handler_list_t&, http::req_wptr_t&, http::resp_wptr_t&);
			virtual bool executeAll(http_handler_list_t&, http::req_wptr_t&, http::resp_wptr_t&);
			virtual bool executeFirst(http_handler_list_t&, http::req_wptr_t&, http::resp_wptr_t&);
			virtual bool executeErrorHandler(http::status_t, http::req_wptr_t&, http::resp_wptr_t&);

			// runall
			/* 
				The post_read_request phase is the first request phase and happens immediately 
				after the request has been read and HTTP headers were parsed. This phase is usually 
				used to do processing that must happen once per request. For example Apache2::Reload is
				usually invoked at this phase to reload modified Perl modules.
			*/
			//static HTTP_HANDLER_RETURN_T defaultPostReadHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The translate phase is used to perform the manipulation of a request's URI. If no 
				custom handler is provided, the server's standard translation rules 
				(e.g., Alias directives, mod_rewrite, etc.) will be used. A PerlTransHandler handler can 
				alter the default translation mechanism or completely override it. This is also a good place to 
				register new handlers for the following phases based on the URI. PerlMapToStorageHandler is
				to be used to override the URI to filename translation.
			*/
			//static HTTP_HANDLER_RETURN_T defaultTranslateUriHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The map_to_storage phase is used to perform the translation of a request's URI into a 
				corresponding filename. If no custom handler is provided, the server will try to walk the 
				filesystem trying to find what file or directory corresponds to the request's URI. Since 
				usually mod_perl handler don't have corresponding files on the filesystem, you will want 
				to shortcut this phase and save quite a few CPU cycles.
			*/
			//static HTTP_HANDLER_RETURN_T defaultStorageMapHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// all
			/*
				When configured inside any container directive, except <VirtualHost>, this handler is an alias 
				for PerlHeaderParserHandler described earlier. Otherwise it acts as an alias for 
				PerlPostReadRequestHandler described earlier.
			*/
			//static HTTP_HANDLER_RETURN_T defaultInitHandler(http::req_wptr_t&, http::resp_wptr_t&);
			// all
			/*
				The header_parser phase is the first phase to happen after the request has been mapped to 
				its <Location> (or an equivalent container). At this phase the handler can examine the request 
				headers and to take a special action based on these. For example this phase can be used to block 
				evil clients targeting certain resources, while little resources were wasted so far.
			*/
			//static HTTP_HANDLER_RETURN_T defaultHeaderParse(http::req_wptr_t&, http::resp_wptr_t&);

			// all
			/*
				The access_checker phase is the first of three handlers that are involved in what's known as 
				AAA: Authentication, Authorization, and Access control.

				This phase can be used to restrict access from a certain IP address, time of the day or 
				any other rule not connected to the user's identity.
			*/
			//static HTTP_HANDLER_RETURN_T defaultAccessHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The check_user_id (authen) phase is called whenever the requested file or directory is 
				password protected. This, in turn, requires that the directory be associated with AuthName, 
				AuthType and at least one require directive.

				This phase is usually used to verify a user's identification credentials. If the credentials 
				are verified to be correct, the handler should return Apache2::Const::OK. Otherwise the handler 
				returns Apache2::Const::HTTP_UNAUTHORIZED to indicate that the user has not authenticated 
				successfully. When Apache sends the HTTP header with this code, the browser will normally pop 
				up a dialog box that prompts the user for login information.
			*/
			//static HTTP_HANDLER_RETURN_T defaultAuthenHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The auth_checker (authz) phase is used for authorization control. This phase requires a 
				successful  authentication from the previous phase, because a username is needed in order 
				to decide whether a user is authorized to access the requested resource.

				As this phase is tightly connected to the authentication phase, the handlers registered for 
				this phase are only called when the requested resource is password protected, similar to the 
				auth phase. The handler is expected to return Apache2::Const::DECLINED to defer the decision, 
				Apache2::Const::OK to indicate its acceptance of the user's authorization, or 
				Apache2::Const::HTTP_UNAUTHORIZED to indicate that the user is not authorized to access the 
				requested document.
			*/
			//static HTTP_HANDLER_RETURN_T defaultAuthzHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The type_checker phase is used to set the response MIME type (Content-type) and sometimes other 
				bits of document type information like the document language.

				For example mod_autoindex, which performs automatic directory indexing, uses this phase to map 
				the filename extensions to the corresponding icons which will be later used in the listing of 
				files.

				Of course later phases may override the mime type set in this phase.
			*/
			//static HTTP_HANDLER_RETURN_T defaultContentTypeHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// all
			/*
					The fixups phase is happening just before the content handling phase. It gives the last 
					chance to do things before the response is generated. For example in this phase mod_env 
					populates the environment with variables configured with SetEnv and PassEnv directives.
			*/
			//static HTTP_HANDLER_RETURN_T defaultFixupHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// first
			/*
				The handler (response) phase is used for generating the response. This is arguably the most 
				important phase and most of the existing Apache modules do most of their work at this phase.
			*/
			//static HTTP_HANDLER_RETURN_T defaultResponseHandler(http::req_wptr_t&, resp_wptr_t&);

			// all
			/*
				The log_transaction phase happens no matter how the previous phases have ended up. If one 
				of the earlier phases has aborted a request, e.g., failed authentication or 404 (file not found) 
				errors, the rest of the phases up to and including the response phases are skipped. But this 
				phase is always executed.

				By this phase all the information about the request and the response is known, therefore the 
				logging handlers usually record this information in various ways (e.g., logging to a flat file 
				or a database).
			*/
			//static HTTP_HANDLER_RETURN_T defaultLogHandler(http::req_wptr_t&, http::resp_wptr_t&);
			
			// all
			/*
				There is no cleanup Apache phase, it exists only inside mod_perl. It is used to execute some 
				code immediately after the request has been served (the client went away) and before the request
				object is destroyed.

				There are several usages for this use phase. The obvious one is to run a cleanup code, for 
				example removing temporarily created files. The less obvious is to use this phase instead of 
				PerlLogHandler if the logging operation is time consuming. This approach allows to free the 
				client as soon as the response is sent.
			*/
			//static HTTP_HANDLER_RETURN_T defaultCleanupHandler(http::req_wptr_t&, http::resp_wptr_t&);

			// void
			//static HTTP_HANDLER_RETURN_T defaultInputFilterHandler(http::req_wptr_t&, http::resp_wptr_t&);
			// void
			//static HTTP_HANDLER_RETURN_T defaultOutputFilterHandler(http::req_wptr_t&, http::resp_wptr_t&);
			
			// preconnect - run upon connection before protocol handoff, all
			// connection - used to process incoming connections, for protocol handlers,
			// allows them to switch out protocols from HTTP to other protocols, id est POP3, FTP, etc


		public:
			handler_mgr_t(workerLog_t&);
			~handler_mgr_t(void);

			virtual bool registerHandler(HTTP_HANDLERS_TYPE_T, handler_t*); //http_handler_t&);
			virtual bool registerHandler(HTTP_HANDLERS_TYPE_T, http_handler_list_t&);
			virtual bool registerHandler(http_handler_array_t&);
			virtual bool registerHandler(error_handler_t*);

			virtual bool executeHandlers(http::status_t, http::req_wptr_t&, http::resp_wptr_t&);
			virtual bool executeHandlers(http::req_wptr_t&, http::resp_wptr_t&);
			virtual bool executeHandlers(HTTP_HANDLERS_TYPE_T, http::req_wptr_t&, http::resp_wptr_t&);
	};
};

#endif
