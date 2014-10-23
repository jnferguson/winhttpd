#include "util.h"

namespace util {
	QString
	getErrorMessage(DWORD e)
	{
		LPWSTR	ptr = nullptr;
		DWORD	ret = 0;

		ret = ::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						nullptr,
						e,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						reinterpret_cast< LPWSTR >(&ptr),
						0,
						nullptr
				);

		if (0 == ret) 
			throw std::runtime_error("errorMsg_t::getErrorMessage() failed to format log message");

		if (nullptr == ptr)
			throw std::runtime_error("errorMsg_t::getErrorMessage(): ::FormatMessage() returned successfully, but yielded a nullptr");

		return QString::fromWCharArray(ptr);
	} 

	void 
	error(QWidget* p, const QString& t, const QString& m, bool gle)
	{
		QMessageBox mb(p);
		QString		e(m + " " + util::getErrorMessage());

		mb.setWindowTitle(t);

		if (true == gle)
			mb.setText(e);
		else
			mb.setText(m);

		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setDefaultButton(QMessageBox::Ok);
		mb.setEscapeButton(QMessageBox::Ok);
		mb.setStandardButtons(QMessageBox::Ok);
		mb.exec();
		return;
	}

	void 
	error(QWidget* p, const QString& t, const QString& m, const QString& i, bool gle)
	{
		QMessageBox mb(p);
		QString		e(i + " " + util::getErrorMessage());

		mb.setWindowTitle(t);
		mb.setText(m);

		if (true == gle)
			mb.setInformativeText(e);
		else
			mb.setInformativeText(i);

		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setDefaultButton(QMessageBox::Ok);
		mb.setEscapeButton(QMessageBox::Ok);
		mb.setStandardButtons(QMessageBox::Ok);
		mb.exec();
		return;
	}

	void 
	error(const QString& t, const QString& m, bool gle)
	{
		QMessageBox mb(nullptr);
		QString		e(m + " " + util::getErrorMessage());

		mb.setWindowTitle(t);

		if (true == gle)
			mb.setText(e);
		else
			mb.setText(m);

		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setDefaultButton(QMessageBox::Ok);
		mb.setEscapeButton(QMessageBox::Ok);
		mb.setStandardButtons(QMessageBox::Ok);
		mb.exec();
		return;
	}

	void 
	error(const QString& t, const QString& m, const QString& i, bool gle)
	{
		QMessageBox mb(nullptr);
		QString		e(i + " " + util::getErrorMessage());

		mb.setWindowTitle(t);
		mb.setText(m);

		if (true == gle)
			mb.setInformativeText(e);
		else
			mb.setInformativeText(i);

		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setDefaultButton(QMessageBox::Ok);
		mb.setEscapeButton(QMessageBox::Ok);
		mb.setStandardButtons(QMessageBox::Ok);
		mb.exec();
		return;
	}
}

