#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QDir>
#include <QStringList>
#include "mwindow.h"
#include "mime.h"

void
initializeMimeOptions(QSettings& s)
{
	mime_descript_t*	c(nullptr);
	std::size_t			i(0);
	QStringList			p;

	s.beginGroup("Plugins/MimeHandler");

	for (i = 0; nullptr != mimes[i].extension && nullptr != mimes[i].type; i++) {
		QString ext		= mimes[i].extension;
		QString type	= mimes[i].type;

		p = ext.split(" ", QString::SplitBehavior::SkipEmptyParts);

		foreach(QString epart, p)
			s.setValue("Options/Extensions/" + epart, type);
	}
	
	s.setValue("Enabled", "true");
	s.setValue("Path", "BuiltIn");

	s.endGroup();

	return;
}

void
initializeAliasOptions(QSettings& s)
{
	s.beginGroup("Plugins/AliasHandler");

	s.setValue("Enabled", "true");
	s.setValue("Path", "BuiltIn");
	s.setValue("Options/image", "/img");
	s.setValue("Options/images", "/img");
	s.setValue("Options/styles", "/css");
	s.setValue("Options/scripts", "/js");
	s.setValue("Options/cgi-bin", "/exec/cgi-bin");

	s.endGroup();

	return;
}

void
initializeFileHandlerOptions(QSettings& s)
{
	s.beginGroup("Plugins/FileHandler");

	s.setValue("Enable", "true");
	s.setValue("Path", "BuiltIn");

	s.setValue("Options/DocumentRoot", QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QDir::separator() + "htdocs"));
	s.setValue("Options/DefaultFile", "index.html index.htm");
	s.setValue("Options/DisplayDirectories", "true");

	s.endGroup();

	return;
}

signed int 
main(signed int ac, char** av)
{
    QCoreApplication::setOrganizationName("NO!SOFT");
    QCoreApplication::setOrganizationDomain("winhttpd.com");
    QCoreApplication::setApplicationName("WINHTTPD");

	static int		r(EXIT_SUCCESS);
	QApplication	a(ac, av);
	mwindow_t*		w(nullptr);
	QSettings		s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD"); 
	QString			fr = s.value("General/FirstRun", "false").toString();

	if (! fr.compare("true", Qt::CaseInsensitive)) {
		s.clear();

		// Qt stupidity work-around.
		// basically there's no way to create a key other than creating a subkey under it
		// so i create it and then delete it as opposed to omitting subkeys named "."
		// under the desired key
		s.setValue("General/.", QVariant(0));
		s.setValue("GUI/.", QVariant(0));
		s.setValue("Controller/.", QVariant(0));
		s.setValue("Worker/.", QVariant(0));
		s.setValue("Plugins/.", QVariant(0));
		s.sync();
		s.remove("General/.");
		s.remove("GUI/.");
		s.remove("Worker/.");
		s.remove("Plugins/.");
		s.remove("Controller/.");


		s.setValue("General/InterpretWeakBindWildcardAsStrong", "true");

		s.setValue("Controller/HTTP/Enable", "true");
		s.setValue("Controller/HTTP/Port", 80);
		s.setValue("Controller/HTTPS/Enable", "true");
		s.setValue("Controller/HTTPS/Port", 443);
		s.setValue("Controller/BindHosts", "*");
		s.setValue("Controller/ManagementURI/Enable", "true");
		s.setValue("Controller/ManagementURI/Host", "127.0.0.1");
		s.setValue("Controller/ManagementURI/Protocol", "HTTPS");
		s.setValue("Controller/ManagementURI/Port", "443");
		s.setValue("Controller/Queue/Length", "4096");
		s.setValue("Controller/Queue/504ResponseVerbosity", "Basic");
		s.setValue("Controller/ManagementURI/Path", "/manage");

		s.setValue("Worker/ManagementURI/AuthRequired", "true");
		s.setValue("Worker/ManagementURI/AuthType", "SSL");
		s.setValue("Worker/ManagementURI/AuthFile", "...");

		initializeFileHandlerOptions(s);
		initializeMimeOptions(s);
		initializeAliasOptions(s);

		s.sync();
	}

	QApplication::setStyle(QStyleFactory::create("Fusion"));

	w = new mwindow_t(nullptr);
	w->show();
	r = a.exec();
	delete w;
	
	return r;
}
