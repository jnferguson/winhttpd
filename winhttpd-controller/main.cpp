
#include <QCoreApplication>
#include <iostream>
#include <stdexcept>

#include "controllerservice.h"

signed int 
main(signed int ac, char** av)
{
	QCoreApplication a(ac, av);

	try {
		QString n("WINHTTPD-Controller");
		QString d("WINHTTPD Controller Service");
		QString e("The Controller daemon for the WINHTTPD web server");
		QString l(QCoreApplication::applicationFilePath());

		controllerService_t* cs = new controllerService_t(n, d, e, l);

		cs->run(*cs);

	} catch (std::exception& e) {
		std::cout << "last errno: " << ::GetLastError() << std::endl;
		std::cout << "fatal exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return a.exec();
}
