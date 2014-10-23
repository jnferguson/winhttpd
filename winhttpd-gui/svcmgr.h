#ifndef SVCMGR_H
#define SVCMGR_H

#include <Windows.h>

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>

#include <QApplication>
#include <QVariant>
#include <QAction>
#include <QButtonGroup>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMovie>

#include "util.h"

class svcMgr_t : public QDialog
{
	Q_OBJECT

	signals:
		void started(void);
		void stopped(void);
		void paused(void);
		void resumed(void);

		void failed(void);
		void success(void);
		void updateStatus(const QString&);

	private slots:
		void start(void);
		void stop(void);
		void pause(void);
		void resume(void);
		void updateLabel(const QString&);


	private:
		QString			m_controller;
		QString			m_worker;

		QVBoxLayout*	m_layout;
		QLabel*			m_label;
		QPushButton*	m_cancel;
		QLabel*			m_loadLabel;
		QMovie*			m_loading;

	protected:
		bool startService(const QString&, bool silent = false);
		bool stopService(const QString&, bool silent = false);
		bool pauseService(const QString&, bool silent = false);
		bool resumeService(const QString&, bool silent = false);

	public:
		svcMgr_t(const QString&, QWidget* p = nullptr);
		~svcMgr_t(void);
		void start_service(void);
		void stop_service(void);
		void pause_service(void);
		void resume_service(void);


	
};

#endif // SVCMGR_H
