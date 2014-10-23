#ifndef HTTP_WORKER_H
#define HTTP_WORKER_H

#define WIN32_LEAN_AND_MEAN

#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>

#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <cstdint>

#include <QObject>
#include <QThread>
#include <QList>
#include <QSettings>

#include "workerlog.h"
#include "http_consts.h"
#include "http_hdr.h"
#include "http_resp.h"
#include "http_req.h"

#define REQUEST_THREAD 0
#define RESPONSE_THREAD 1

#define REQUEST_MUTEX 0
#define RESPONSE_MUTEX 1
#define OBJECT_MUTEX 2

namespace http {
	/*
	typedef USHORT status_t;
	typedef std::vector< uint8_t > resp_vec_t;
	typedef const uint8_t* resp_ptr_t;
	typedef HTTP_REQUEST_ID id_t;

	typedef HTTP_CACHE_POLICY cache_t;
	typedef std::shared_ptr< HTTP_RESPONSE > response_shared_ptr_t;
	typedef std::shared_ptr< HTTP_REQUEST > request_shared_ptr_t;

	typedef struct {
		id_t					id;
		response_shared_ptr_t	response;
		cache_t					cache;
		http::hdr_t				hdrs;
	} response_t;

	typedef struct {
		id_t					id;
		request_shared_ptr_t	request;
		cache_t					cache;
		http::hdr_t				hdrs;
	} request_t;

	typedef std::shared_ptr< HTTP_REQUEST > request_sptr_t;
	typedef std::shared_ptr< response_t > response_sptr_t;


	typedef QList< request_sptr_t > request_queue_t;
	typedef QList< response_sptr_t > response_queue_t;
	*/

	class worker_t : public QObject
	{
		Q_OBJECT

	private:
		std::mutex						m_mutex[3];
		HANDLE							m_rqueue;
		workerLog_t&					m_log;
		bool							m_stop;
		std::thread*					m_threads[2];
		bool							m_logConnections;

		QList< http::req_sptr_t >		m_request_queue;
		QList< http::req_sptr_t >		m_disconnect_queue;
		QList< http::resp_sptr_t >		m_response_queue;


	protected:
		virtual void pollRequests(void);
		virtual void pollResponses(void);

		virtual http::req_sptr_t getRequest(void);
		virtual bool putResponse(http::resp_sptr_t);
		virtual void logHttpRequest(http::req_sptr_t);


	public:
		worker_t(workerLog_t&, QObject* p = nullptr);
		virtual ~worker_t(void);

		void setStop(bool);
		bool getStop(void);

		bool start(void);
		void stop(void);

		bool inputQueueIsEmpty(void);
		void putRequestOnInputQueue(http::req_sptr_t);
		http::req_sptr_t getRequestFromInputQueue(void);


		bool outputQueueIsEmpty(void);
		http::resp_sptr_t getResponseFromOutputQueue(void);
		void putResponseOnOutputQueue(http::resp_sptr_t);

		void waitForClientDisconnect(http::req_sptr_t);

	};
};
#endif // HTTP_WORKER_H
