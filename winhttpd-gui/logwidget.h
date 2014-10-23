#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QWidget>
#include <QTableWidgetItem>
#include <QThread>
#include <QAction>

#include <stdexcept>
#include <mutex>
#include <algorithm>

#include "eventlog.h"

class logWidget_t : public QWidget
{
	Q_OBJECT

	public slots:
		void updateTable(eventLogEntry_t*);

	private slots:
		void recvErrorMessage(const QString&, const QString&, bool);
		void clearTable(void);
		void removeTable(void);

	private:
		std::mutex		m_mutex;
		QVBoxLayout*	m_layout;
		QTableWidget*	m_table;
		eventLog_t*		m_log;

		QAction*		m_clear;
		QAction*		m_remove;

	protected:

	public:
		logWidget_t(QWidget* p = nullptr);
		~logWidget_t(void);
};

#endif // LOGWIDGET_H
