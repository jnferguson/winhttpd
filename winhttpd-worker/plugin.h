#ifndef HAVE_PLUGIN_H
#define HAVE_PLUGIN_H

#include <QCoreApplication>
#include <QLibrary>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSettings>

#include <stdexcept>
#include <mutex>
#include <vector>
#include <map>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <functional>
#include <array>
#include <memory>

#include "http_handler.h"
#include "http_req.h"
#include "http_resp.h"


class plugin_t
{

	private:
		typedef std::unique_ptr< QLibrary > lib_uptr_t;

		std::mutex					m_mutex;
		lib_uptr_t					m_lib;
		QString						m_name;
		QString						m_file;
		http::handler_array_sptr_t	m_handlers;

	protected:
		void init(QString&, QStringList&);
		static bool setLibraryPaths(QStringList&);
		static QStringList getLibraryPaths(void);

		http::plugin_getName_t getNameResolve(void);

	public:
		plugin_t(QString&, QStringList&);
		plugin_t(QString, QStringList&);
		~plugin_t(void);

		http::http_handler_list_t& getHandlerList(http::HTTP_HANDLERS_TYPE_T);
		http::handler_array_sptr_t getAllHandlers(void);
};

#endif
