#include "file_handler.h"

namespace http {

	file_handler_t::file_handler_t(workerLog_t& l) : handler_t(l)
	{
		QSettings	s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

		s.beginGroup("Plugins/FileHandler");

		if (!s.value("Enable", "false").toString().compare("false", Qt::CaseInsensitive)) {
			m_log.error("file_handler_t::file_handler_t(): Plugin initialized when disabled in configuration", eventlog_cat_t::CAT_WORKER, false);
			throw std::runtime_error("file_handler_t::file_handler_t(): Plugin initialized when disabled in configuration");
			return;
		}
			
		m_name = s.value("Name", "FileHandler").toString();

		m_root = s.value("Options/DocumentRoot", QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QDir::separator() + "htdocs")).toString();
		m_root += QDir::separator();
		m_root = QDir::toNativeSeparators(m_root);
		
		m_default	= s.value("Options/DefaultFile", QStringList("index.html")).toStringList();
		m_dirs		= s.value("Options/DisplayDirectories", false).toBool();


		s.endGroup();

		m_log.info("file_handler: plugin initialized");

		return;
	}

	HTTP_HANDLER_RETURN_T
	file_handler_t::operator()(http::req_sptr_t& req, http::resp_sptr_t& resp)
	{
		QString	p("");

		if (nullptr == req.get() || nullptr == resp.get())
			throw std::runtime_error("file_handler_t::operator(): Received nullptr for one or more parameter.");

		p = normalizePath(req->getAbsolutePath());

		// ?? fixes ?? almost certainly yields a 404 every time - FIXME
		//if (true == hasDirTraversal(p)) 
		//	p = QDir::toNativeSeparators(m_root + "\\" + p); 

		if ( !QFile(p).exists()) 
			resp->setStatus(404);

		else {
			QFile		fil(p);
			QByteArray	buf;
			qint64		len(0);
			QString		str("");


			if (false == fil.open(QIODevice::ReadOnly)) 
				resp->setStatus(500);
			
			else {

				do {
					buf += fil.read(fil.size());
					len = buf.length();
				} while (len < fil.size());

				fil.close();

				resp->setStatus(200);
				resp->setBody(reinterpret_cast< uint8_t* >(buf.data()), buf.size());
			}
		}

		return HTTP_HANDLER_RETURN_OKAY;

	}

	HTTP_HANDLER_RETURN_T
	file_handler_t::operator()(http::req_wptr_t& req, http::resp_wptr_t& resp)
	{
		http::resp_sptr_t	respPtr = resp.lock();
		http::req_sptr_t	reqPtr = req.lock();

		if (respPtr && reqPtr)
			return operator()(reqPtr, respPtr);


		return HTTP_HANDLER_RETURN_DECLINED;
	}

	QString 
	file_handler_t::normalizePath(QString& inpath)
	{
		QDir	direct(m_root + "\\" + inpath);

		return direct.toNativeSeparators(direct.absolutePath());
	}

	/*
	 * The HTTP API appears to handle this for us such that the
	 * URI we receive has always been canonicalized to a relative
	 * path.
	 *
	 * At some point I may decide this is entirely superfluous and
	 * remove this functionality, but for the time being I'd rather
	 * be safe with a minimal performance impact. This module isn't
	 * really meant to be used anyway as ideally everything is 
	 * dynamically generated and this is mostly just here to 
	 * satisfy the requirement and build standard HTTP server 
	 * functionality.
	 * 
	 * -jf 07-june-2014
	*/
	bool 
	file_handler_t::hasDirTraversal(QString& inpath)
	{
		inpath = normalizePath(inpath);

		if (!inpath.startsWith(m_root, Qt::CaseInsensitive))
			return true; 

		return false;
	}
}
