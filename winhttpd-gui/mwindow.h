#ifndef MWINDOW_H
#define MWINDOW_H


#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

#include "menubar.h"
#include "statustab.h"
#include "connectiontab.h"
#include "configtab.h"
#include "usertab.h"
#include "hosttab.h"

class mwindow_t : public QMainWindow
{
	Q_OBJECT

	private:
		QWidget*			m_central;
		QStatusBar*			m_status;
		menubar_t*			m_menu;
		QHBoxLayout*		m_layout; 
		QTabWidget*			m_tabs;
		statusTab_t*		m_statusTab;
		connectionTab_t*	m_connectlogTab;
		configTab_t*		m_configTab;
		userTab_t*			m_userTab;
		hostTab_t*			m_hostTab;

	protected:
	public:
		mwindow_t(QWidget* p = nullptr);
		~mwindow_t(void);

};

#endif // MWINDOW_H
