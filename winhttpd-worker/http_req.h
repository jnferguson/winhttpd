#ifndef HTTP_REQ_H
#define HTTP_REQ_H

#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>

#include <QObject>
#include <QVector>

#include <cstdint>
#include <memory>
#include <mutex>

#include "http_consts.h"
#include "http_hdr.h"
#include "workerlog.h"

namespace http {

	class req_t {

		private:
			std::mutex						m_mutex;
			workerLog_t&					m_log;
			http::id_t						m_id;
			http::connection_id_t			m_connectId;
			http::url_context_t				m_urlContext;
			http::version_t					m_version;
			http::verb_t					m_verb;
			http::transport_addr_t			m_addrs;
			http::raw_connection_id_t		m_rawConnectId;
			http::cert_info_t				m_ssl;
			std::size_t						m_totalLength;
			// XXX JF TODO HTTP_REQUEST_V2 / PHTTP_REQUEST_INFO 

			http::hdr_t						m_headers;
			http::resp_vec_t				m_body;
	
			QString							m_laddr;
			QString							m_lport;
			QString							m_raddr;
			QString							m_rport;
			QString							m_unknownVerb;
			QString							m_rawUrl;
			QString							m_path;
			QString							m_url;
			QString							m_query;
			QString							m_host;
	
		protected:
			void initializeVerb(HTTP_REQUEST&);
			void initializeUrl(HTTP_REQUEST&);
			void initializeAddress(HTTP_REQUEST&);
			void initializeHeaders(HTTP_REQUEST&);
			void initializeBody(HTTP_REQUEST&);
			void initializeSsl(HTTP_REQUEST&);
			QString addressToString(SOCKADDR&);
			QString portToString(SOCKADDR&);
	
		public:
			req_t(HTTP_REQUEST*, workerLog_t&);
			~req_t(void);

			http::id_t getId(void);
			http::resp_vec_t getBody(void);
			http::hdr_t getHeaders(void);

			QString getVerbString(void);
			QString getRawURL(void);
			QString getAbsolutePath(void);
			QString getURL(void);
			QString getHost(void);
			QString getQuery(void);

			bool	hasQueryString(void);

			QString getLocalAddress(void) { return m_laddr; }
			QString getRemoteAddress(void) { return m_raddr; }
			QString getLocalPort(void) { return m_lport; }
			QString getRemotePort(void) { return m_rport; }
	
			http::connection_id_t getConnectionId(void);
			QString getVersionString(void);
			unsigned short getVersionMajor(void);
			unsigned short getVersionMinor(void);
			std::size_t getTotalLength(void);


			QString toString(void);

			/*
			XXX JF TODO:
			http::transport_addr_t			m_addrs;
			http::raw_connection_id_t		m_rawConnectId;
			http::cert_info_t				m_ssl;
			*/


			void setId(http::id_t);
			void addHeader(QString&, QString&);
			void addHeader(QString k, QString v) { m_headers.addHeader(k, v); return; }
			void setHeader(QString&, QString&);
			//void getHeader(QString&);
			//bool hasHeader(QString&);

			void setBody(http::resp_vec_t&);
			void setBody(uint8_t*, std::size_t);
			void setBody(QString&);

			HTTP_REQUEST* toHttpResponse(void);
	};

	typedef std::shared_ptr< req_t > req_sptr_t;
	typedef std::unique_ptr< req_t > req_uptr_t;
	typedef std::weak_ptr< req_t > req_wptr_t;
} 
#endif
