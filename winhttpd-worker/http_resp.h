#ifndef HTTP_RESP_H
#define HTTP_RESP_H

#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>

#include <QObject>
#include <QVector>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "http_consts.h"
#include "http_hdr.h"
#include "workerlog.h"

namespace http {

	class chunk_t {
		private:
			HTTP_DATA_CHUNK*	m_chunk;
			uint8_t*			m_buf;
			std::size_t			m_len;

		protected:
		public:
			chunk_t(uint8_t* b, std::size_t l) : m_chunk(new HTTP_DATA_CHUNK), m_buf(new uint8_t[l + 1]), m_len(l)
			{
				if (nullptr == m_chunk)
					throw std::runtime_error("http::chunk_t::chunk_t(): Unexpected error while handling HTTP_DATA_CHUNK");

				::memcpy_s(reinterpret_cast<void *>(m_buf), l + 1, b, l);

				m_chunk->DataChunkType = HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromMemory;
				m_chunk->FromMemory.pBuffer = m_buf;
				m_chunk->FromMemory.BufferLength = m_len;
				return;
			}

			~chunk_t(void)
			{
				m_chunk->FromMemory.BufferLength = 0;
						
				delete m_chunk->FromMemory.pBuffer;
				m_chunk->FromMemory.pBuffer = nullptr;

				delete m_chunk;
				m_chunk = nullptr;

				return;
			}

			HTTP_DATA_CHUNK*
			toChunk(void) 
			{
				return m_chunk;
			}

	};

	class resp_t
	{

		private:
			std::mutex						m_mutex;
			workerLog_t&					m_log;
			http::id_t						m_id;
			http::cache_t					m_cache;
			http::hdr_t						m_headers;
			http::resp_vec_t				m_body;
			http::status_t					m_status;

			/* std::shared_ptr< HTTP_RESPONSE >	m_resp; */
		protected:
			QString getStatusString(void);

		public:
			resp_t(workerLog_t&);
			~resp_t(void);


			http::id_t getId(void);
			http::status_t getStatus(void);
			http::cache_t getCache(void);
			http::resp_vec_t getBody(void);
			http::hdr_t	getHeaders(void);

			void setId(http::id_t);
			void setStatus(http::status_t);
			void setCache(http::cache_t);


			void addHeader(QString&, QString&);
			//void addHeader(QString k, QString v) { m_headers.addHeader(k,v); return; }
			void setHeader(QString&, QString&);

	
			void setBody(http::resp_vec_t&);
			void setBody(uint8_t*, std::size_t);
			void setBody(QString&);

			HTTP_RESPONSE* toHttpResponse(void);
	};

	typedef std::shared_ptr< resp_t > resp_sptr_t;
	typedef std::unique_ptr< resp_t > resp_uptr_t;
	typedef std::weak_ptr< resp_t > resp_wptr_t;
}; 

#endif // HTTP_RESP_H
