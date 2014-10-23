#ifndef HTTP_CONSTS_H
#define HTTP_CONSTS_H

#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>

#include <QVector>
#include <QByteArray>

#include <cstdint>
#include <memory>

#include "http_hdr.h"

namespace http {
//namespace HTTP_CONSTS {
	typedef USHORT status_t;
	typedef QByteArray resp_vec_t;
	//typedef QVector< uint8_t > resp_vec_t;
	typedef const uint8_t* resp_ptr_t;
	typedef HTTP_REQUEST_ID id_t;

	typedef HTTP_CACHE_POLICY cache_t;
	typedef HTTP_CONNECTION_ID connection_id_t;
	typedef HTTP_RAW_CONNECTION_ID  raw_connection_id_t;
	typedef HTTP_URL_CONTEXT   url_context_t;
	typedef HTTP_VERSION       version_t;
	typedef HTTP_VERB          verb_t;
	typedef HTTP_TRANSPORT_ADDRESS transport_addr_t;
	typedef HTTP_SSL_INFO ssl_info_t;
	typedef HTTP_SSL_CLIENT_CERT_INFO ssl_client_cert_info_t;

	typedef std::shared_ptr< HTTP_RESPONSE > response_shared_ptr_t;
	typedef std::shared_ptr< HTTP_REQUEST > request_shared_ptr_t;

	typedef struct {
		bool hasClientCertificate;

		struct {
			unsigned short	certSize;
			unsigned short	keySize;
			QString			issuer;
			QString			subject;
		} svr;

		struct {
			unsigned long	flags;
			QString			certificate;
			// todo: token HTTP_SERVICE_CONFIG_SSL_FLAG_USE_DS_MAPPER 
		} client;

	} cert_info_t;

	/*typedef struct {
		id_t					id;
		response_shared_ptr_t	response;
		cache_t					cache;
		http::hdr_t				hdrs;
	} http_response_t;

	typedef struct {
		id_t					id;
		request_shared_ptr_t	request;
		cache_t					cache;
		http::hdr_t				hdrs;
	} http_request_t;

	typedef std::shared_ptr< http_request_t > request_sptr_t;
	typedef std::shared_ptr< http_response_t > response_sptr_t;*/

}

#endif