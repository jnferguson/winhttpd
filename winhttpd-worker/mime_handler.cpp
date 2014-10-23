#include "mime_handler.h"

namespace http {

	mime_handler_t::mime_handler_t(workerLog_t& l) : handler_t(l), m_log(l)
	{
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");
		QStringList e;

		s.beginGroup("Plugins/MimeHandler");

		if ( !s.value("Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_log.error("mime_handler_t::mime_handler_t(): Plugin initialized when explicitly disabled", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("mime_handler_t::mime_handler_t(): Plugin initialized when explicitly disabled");
		}

		m_name			= s.value("Name", "MimeHandler").toString();
		m_defaultType	= s.value("Options/DefaultType", "text/plain").toString();

		s.beginGroup("Options/Extensions");
		e = s.allKeys();
		s.endGroup();

		foreach(QString k, e) 
			m_types[k] = s.value("Options/Extensions/" + k, "").toString();

		e = s.value("Options/DefaultFile", QStringList("index.html")).toStringList();

		foreach(QString d, e) {
			QStringList t = d.split(".", QString::SplitBehavior::SkipEmptyParts);
			d = t.last();
			m_default.push_back(d);
		}

		s.endGroup();
		m_log.info("mime_handler: plugin initialized");

		return;
	}


	mime_handler_t::~mime_handler_t(void)
	{
		m_types.clear();
		return;
	}

	HTTP_HANDLER_RETURN_T 
	mime_handler_t::operator()(http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		http::resp_sptr_t	respPtr = resp.lock();
		http::req_sptr_t	reqPtr	= req.lock();

		if (respPtr && reqPtr)
			return operator()(reqPtr, respPtr);


		return HTTP_HANDLER_RETURN_DECLINED;
	}

	HTTP_HANDLER_RETURN_T 
	mime_handler_t::operator()(http::req_sptr_t& req, http::resp_sptr_t& resp)
	{
		QString		type("Content-Type");
		QString		uri;
		QStringList	ext;

		if (nullptr == req.get() || nullptr == resp.get())
			return HTTP_HANDLER_RETURN_DECLINED; // 500

		uri = req->getAbsolutePath();

		// XXX JF FIXME - requests lacking a file / default to index.html
		// issue is if the default list is like index.html index.php index.pl
		// where its not possible to determine which mime type we will ultimately
		// serve...

		if (uri.contains(".")) {
			ext = uri.split(".", QString::SplitBehavior::SkipEmptyParts);
			
			if (m_types.end() == m_types.find(ext.last()))
				resp->addHeader(type, m_defaultType);
			else
				resp->addHeader(type, m_types[ext.last()]);

		} else
			resp->addHeader(type, m_defaultType);

		return HTTP_HANDLER_RETURN_OKAY;
	}

	/*
	QString 
	mime_handler_t::getMimeTypeByExtension(QString& t)
	{
		for (auto itr = m_types.begin(); itr != m_types.end(); itr++) {

		}

	}

	QString 
	mime_handler_t::getExtensionByMime(QString&)
	{

	}
	*/
}
