#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>
#include <QWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QCoreApplication>

class menubar_t : public QMenuBar
{
	Q_OBJECT
	
	signals:
		void exitRaised(void);
		void cutRaised(void);
		void copyRaised(void);
		void pasteRaised(void);
		void selectAllRaised(void);
		void helpRaised(void);
		void aboutRaised(void);

		void startServiceRaised(void);
		void stopServiceRaised(void);
		void pauseServiceRaised(void);
		void resumeServiceRaised(void);

	private:
		QMenu*			m_menuFile;
		QMenu*			m_menuFileService;
		QMenu*			m_menuEdit;
		QMenu*			m_menuHelp;

		QAction*		m_startService;
		QAction*		m_stopService;
		QAction*		m_pauseService;
		QAction*		m_resumeService;

		QAction*		m_menuSettings;
		QAction*		m_menuExit;
		QAction*		m_editCut;
		QAction*		m_editCopy;
		QAction*		m_editSelectAll;
		QAction*		m_editPaste;
		QAction*		m_helpHelp;
		QAction*		m_helpAbout;

	protected:
	protected slots:
		void settingsTriggered(void);
		void exitTriggered(void);
		void cutTriggered(void);
		void copyTriggered(void);
		void pasteTriggered(void);
		void selectAllTriggered(void);
		void helpTriggered(void);
		void aboutTriggered(void);

		void startServiceTriggered(void);
		void stopServiceTriggered(void);
		void pauseServiceTriggered(void);
		void resumeServiceTriggered(void);

	public slots:
		void setServiceState(QString&);
	
	public:
		menubar_t(QMainWindow* p = nullptr);
		~menubar_t(void);

		

};

#endif // MENUBAR_H
