
#include <QCoreApplication>

#include <iostream>
#include <stdexcept>

#include "workerservice.h"

signed int 
main(signed int ac, char** av)
{
	QCoreApplication a(ac, av);
	workerService_t* w(nullptr);

	try {
		QString n("WINHTTPD-Worker");
		QString d("WINHTTPD Worker Service");
		QString e("The Worker daemon for the WINHTTPD web server");
		QString l(QCoreApplication::applicationFilePath());

		w = new workerService_t(n,d,e,l);
		w->run(*w);

	} catch(std::exception& e) {
		std::cout << "last errno: " << ::GetLastError() << std::endl;
		std::cout << "fatal exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return a.exec();
}
