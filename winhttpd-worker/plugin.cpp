#include "plugin.h"

plugin_t::plugin_t(QString& f, QStringList& p) : m_lib(nullptr), m_file(f), m_name(""), m_handlers(nullptr)
{
	this->init(f,p);
	return;
}

plugin_t::plugin_t(QString f, QStringList& p) : m_lib(nullptr), m_file(f), m_name(""), m_handlers(nullptr) 
{
	this->init(f,p);
	return;
}

plugin_t::~plugin_t(void)
{
	std::lock_guard< std::mutex > l(m_mutex);

	if (nullptr != m_lib && m_lib->isLoaded()) {
		m_lib->unload();
		m_lib = nullptr;
	}

	m_handlers = nullptr;
	return;
}

void
plugin_t::init(QString& f, QStringList& p)
{
	std::lock_guard< std::mutex >	l(m_mutex); 
	QSettings						s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
	http::plugin_getName_t			gn(nullptr);
	http::plugin_getHandlers_t		gh(nullptr);
	http::plugin_initHandler_t		ih(nullptr);

	QStringList						sysp;
	QString							err("");

	m_lib = plugin_t::lib_uptr_t(new QLibrary());

	if (! f.endsWith(".dll", Qt::CaseInsensitive))
		f += ".dll";

	sysp = plugin_t::getLibraryPaths();

	if (false == plugin_t::setLibraryPaths(p))
		throw std::runtime_error("plugin_t::plugin_t(): Failed to set application specific DLL directory paths");

	m_lib->setFileName(f);
	m_lib->setLoadHints(QLibrary::ResolveAllSymbolsHint);

	if (false == m_lib->load() || false == m_lib->isLoaded()) {
		plugin_t::setLibraryPaths(sysp);
		err = "plugin_t::plugin_t(): Failed to load plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());
	}

	if (false == plugin_t::setLibraryPaths(sysp)) 
		throw std::runtime_error("plugin_t::plugin_t(): Failed to restore system DLL directory paths");

	gn = reinterpret_cast< QString(*)(void) >(m_lib->resolve("getPluginName"));

	if (nullptr == gn) {
		err = "plugin_t::plugin_t(): Failed to resolve symbol 'getPluginName' from plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());
	}

	m_name = gn();

	ih = reinterpret_cast< bool(*)(QString&, QSettings&) >(m_lib->resolve("initializePlugin"));

	if (nullptr == ih) {
		err = "plugin_t::plugin_t(): Failed to resolve symbol 'initializePlugin' from plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());		
	}

	s.beginGroup("plugins/" + m_name);

	if (false == ih(m_name, s)) {
		err = "plugin_t::plugin_t(): Failed to initialize plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());	
	}

	gh = reinterpret_cast< http::http_handler_array_t(*)(void) >(m_lib->resolve("getPluginHandlers"));

	if (nullptr == gh) {
		err = "plugin_t::plugin_t(): Failed to resolve symbol 'getPluginHandlers' from plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());	
	}

	m_handlers = http::handler_array_sptr_t(new http::http_handler_array_t(gh()));

	if (nullptr == m_handlers) {
		err = "plugin_t::plugin_t(): Failed to obtain handler array from plugin: " + f;
		throw std::runtime_error(err.toStdString().c_str());
	}

	return;
}

bool 
plugin_t::setLibraryPaths(QStringList& p)
{
	QCoreApplication::setLibraryPaths(p);
	return true;
}

QStringList 
plugin_t::getLibraryPaths(void)
{
	return QCoreApplication::libraryPaths();
}

http::http_handler_list_t& 
plugin_t::getHandlerList(http::HTTP_HANDLERS_TYPE_T t)
{
	if (nullptr == m_handlers)
		throw std::runtime_error("plugin_t::getHandlerList(): Attempted to retrieve plugin handlers while object was in an invalid state (nullptr)");

	return m_handlers->at(t);
}

http::handler_array_sptr_t 
plugin_t::getAllHandlers(void)
{
	if (nullptr == m_handlers)
		throw std::runtime_error("plugin_t::getHandlerList(): Attempted to retrieve plugin handlers while object was in an invalid state (nullptr)");

	return m_handlers;
}
