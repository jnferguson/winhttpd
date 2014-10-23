#ifndef HTTP_HDR_H
#define HTTP_HDR_H

#define WIN32_LEAN_AND_MEAN

#include <SDKDDKVer.h>
#include <Windows.h>
#include <http.h>

#include <QString>
#include <QVector>

#include <array>
#include <mutex>
#include <limits>

#include "workerlog.h"

namespace http {

	class hdr_t
	{
		private:
			typedef struct {
				HTTP_HEADER_ID	id;
				QString			name;
			} hdr_entry_t;

			typedef struct {
				bool				isKnown;
				HTTP_HEADER_ID		id;
				QString				key;
				QString				value;
			} vec_entry_t;

			static std::array< hdr_entry_t, 42 > m_resphdr;
			static std::array< hdr_entry_t, 31 > m_reqhdr;

			QVector< vec_entry_t > m_hdrs;
			//std::mutex					m_mutex;
			workerLog_t&				m_log;
	
		protected:
			void addEntry(vec_entry_t&, bool overwrite);

		public:
			hdr_t(workerLog_t&);
			~hdr_t(void);

			static HTTP_HEADER_ID getHeaderByName(QString&);
			static QString getHeaderById(HTTP_HEADER_ID&);

			void addHeader(HTTP_HEADER_ID, QString, bool overwrite = false);
			void addHeader(QString, QString, bool overwrite = false);
			void setHeader(HTTP_HEADER_ID, QString);
			void setHeader(QString, QString);

			HTTP_RESPONSE_HEADERS* getResponseHeaders(void);
			HTTP_REQUEST_HEADERS* getRequestHeaders(void);

			void clearHeaders(void);

			void destroyResponseHeaders(HTTP_RESPONSE_HEADERS*);
			static void destroyRequestHeaders(HTTP_REQUEST_HEADERS*);
	};
}; 
#endif // HTTP_HDR_H
