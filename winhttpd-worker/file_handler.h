#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>

#include "workerlog.h"
#include "http_handler.h"

namespace http {

	class file_handler_t : public handler_t {
		private:
			QString		m_root;
			QStringList m_default;
			bool		m_dirs;
			

		protected:
			inline QString normalizePath(QString&);
			inline bool hasDirTraversal(QString&);

		public:
			file_handler_t(workerLog_t&);
			virtual ~file_handler_t(void) {}
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_wptr_t&, http::resp_wptr_t&);
			virtual HTTP_HANDLER_RETURN_T operator()(http::req_sptr_t&, http::resp_sptr_t&);
		};
}

#endif