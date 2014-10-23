#ifndef MIME_HANDLER_H
#define MIME_HANDLER_H

#include <QString>
#include <QSettings>
#include <QMap>
#include <QStringList>
#include <stdexcept>
#include <functional>

#include "http_handler.h"
#include "workerlog.h"

namespace http {

	//static std::function< QString(QString&) > getMimeType = &mime_handler_t::getMimeTypeByExtension;
	//static std::function< QString(QString&) > getExtension = &mime_handler_t::getExtensionByMime;

	class mime_handler_t : public handler_t {

		private:
			QMap< QString, QString >	m_types;
			QStringList					m_default;
			QString						m_defaultType;
			workerLog_t&				m_log;

		protected:
		public:
			mime_handler_t(workerLog_t&);
			virtual ~mime_handler_t(void);
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_wptr_t&, http::resp_wptr_t&);
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_sptr_t&, http::resp_sptr_t&);

			//static QString getMimeTypeByExtension(QString&);
			//static QString getExtensionByMime(QString&);
	};
}

#endif
