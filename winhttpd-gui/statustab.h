#ifndef STATUSTAB_H
#define STATUSTAB_H

#include <Windows.h>

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>
#include <QListWidget>
#include <QTimer>

#include <stdexcept>
#include <thread>

#include "statuslight.h"
#include "svcmgr.h"
#include "util.h"
#include "logwidget.h"
#include "eventlog.h"

class statusTab_t : public QWidget
{
	Q_OBJECT

	signals:
		void setStatus(statusLight_t::status_state_t);
		void updateStatus(QString&);

	private:
		QTimer*							m_timer;
		QVBoxLayout*					m_layout; 
		QGroupBox*						m_statBox;
		QHBoxLayout*					m_hlayout; 
		statusLight_t*					m_light;
		statusLight_t::status_state_t	m_state;
		QComboBox*						m_combo;
		QListWidget*					m_list;
		QGroupBox*						m_logBox;
		QVBoxLayout*					m_vlayout; 
		logWidget_t*					m_log;

	protected:
		void initComboBox(void);

		void updateLightStatus(void);
		DWORD getServiceStatus(QString);

		bool startServices(void);
		bool stopServices(void);
		bool pauseServices(void);
		bool resumeServices(void);

		bool startService(QString, bool silent = false);
		bool stopService(QString);
		bool pauseService(QString);
		bool resumeService(QString);
		bool stopDependencies(QString, SC_HANDLE, SC_HANDLE);
		bool pauseDependencies(QString);
		bool resumeDependencies(QString);

	protected slots:
		void comboActivated(const QString&);

	public slots:
		void start_svc(void);
		void stop_svc(void);
		void pause_svc(void);
		void resume_svc(void);

	public:
		statusTab_t(QWidget* p = nullptr);
		~statusTab_t(void);

};

#endif // STATUSTAB_H
