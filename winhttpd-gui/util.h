#ifndef UTIL_H
#define UTIL_H

#include <Windows.h>

#include <QObject>
#include <QString>
#include <QMessageBox>

namespace util {
	QString getErrorMessage(const DWORD e = ::GetLastError());
	void error(QWidget*, const QString&, const QString&, bool gle = true);
	void error(QWidget*, const QString&, const QString&, const QString&, bool gle = true);
	void error(const QString&, const QString&, bool gle = true);
	void error(const QString&, const QString&, const QString&, bool gle = true);


}

#endif // UTIL_H
