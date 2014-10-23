#ifndef QSERVICE_H
#define QSERVICE_H

#define WIN32_LEAN_AND_MEAN
#include <QObject>
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QCoreApplication>

#include <stdexcept>

#include <Windows.h>

#include "eventlog.h"

class qservice_t : public QObject
{
	Q_OBJECT
	
	public:
		typedef enum { QSERVICE_STARTING = 0, QSERVICE_RUNNING = 1, QSERVICE_STOPPED = 2, QSERVICE_PAUSED = 3, QSERVICE_RESUME = 4, QSERVICE_INVALID = 5 } qservice_state_t;

	private:
		void				start(DWORD, LPTSTR*);
		DWORD				ctrlh(DWORD, DWORD, LPVOID);

		static DWORD WINAPI	ServiceCtrl(DWORD, DWORD, LPVOID, LPVOID);
		static VOID WINAPI	ServiceMain(DWORD, LPTSTR*);

		static qservice_t*		m_instance;
		QString					m_svcname;
		QString					m_dispname;
		QString					m_path;

		DWORD					m_access;		
		DWORD					m_type;		
		DWORD					m_start;			
		DWORD					m_error;		
		LPCTSTR					m_loadgroup;	
		DWORD					m_tagid;	
		QVector< QString >		m_dependencies;
		QString					m_deps;
		QString					m_user;
		QString					m_pass;
		QString					m_description;
		DWORD					m_checkpoint;
		SERVICE_STATUS_HANDLE	m_handle;
		SERVICE_STATUS			m_status;
		eventLog_t*				m_log;

	protected:
		static qservice_t& instance(void);
		virtual BOOL reportStatus(DWORD, DWORD w = 3000, DWORD e = 0); 
		bool stopDependencies(SC_HANDLE m, SC_HANDLE h);


		DWORD getAccessRights(void);
		DWORD getServiceType(void);
		DWORD getStartType(void);
		DWORD getErrorControlType(void);
		QString& getDependencies(void);
		QString& getUser(void);
		QString& getPassword(void);
		virtual eventLog_t& getLog(void);

		LPWSTR intGetDependencies(void);
		
		virtual void setState(qservice_state_t) = 0;
		virtual qservice_t::qservice_state_t getState(void) = 0;
		virtual void fsm(void) = 0;

	public:
		/// <c>qservice_t::qservice_t(QString n, QString d, QString l, QObject* p = nullptr)</c>
		/// <summary>
		///		The qservice_t class is a generic wrapper to most functionality requisite for 
		///		installing, running and other associated functionality of a Windows service.
		///		<remarks>
		///			<para>
		///				<example>
		///					<para>Example:</para>
		///					<code>
		///						qservice_t q("myService", "My Service", "C:\\myService.exe", elogObj, nullptr);
		///					</code>
		///				</example>
		///			</para>
		///			<para>The <paramref name="n" /> parameter specifies the service name</para>
		///			<para>The <paramref name="d" /> parameter specifies the display name of the service</para>
		///			<para>The <paramref name="e" /> parameter specifies the description for the service</para>
		///			<para>The <paramref name="l" /> parameter specifies the path to the services executable file</para>
		///			<para>The <paramref name="p" /> parameter is a pointer to the Qt parent object</para>
		///		</remarks>
		/// </summary>
		qservice_t(QString, QString, QString, QString, QObject* p = nullptr);
		virtual ~qservice_t(void);	

		bool install(void);
		bool remove(void);
		bool start(void);
		bool stop(void);
		bool pause(void);
		bool resume(void);

		virtual void onStart(DWORD, LPTSTR*)	= 0;
		virtual void onStop(void)				= 0;
		virtual void onPause(void)				= 0;
		virtual void onContinue(void)			= 0;
		virtual void onShutdown(void)			= 0;

		virtual LPSERVICE_MAIN_FUNCTION getServiceMainPointer(void) const;
		inline const QString& getServiceName(void) const;
		static void run(qservice_t&);

		void setAccessRights(DWORD);
		void setServiceType(DWORD);
		void setStartType(DWORD);
		void setErrorControlType(DWORD);
		void setDependencies(QVector< QString >&);
		void addDependencies(QString);
		void setUser(QString);
		void setPassword(QString);
		void setDescription(QString);

		bool pauseDependencies(void);
		bool resumeDependencies(void);
		bool stopDependencies(void);
};

#endif // QSERVICE_H
