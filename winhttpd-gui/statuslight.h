#ifndef STATUSLIGHT_H
#define STATUSLIGHT_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QString>

#include <mutex>
#include <array>
#include <memory>

#ifndef HAVE_LIGHT_STATUS_DEFINES
#define HAVE_LIGHT_STATUS_DEFINES

#define LIGHT_SIZE 11
#define INDEX_GREEN 0
#define INDEX_RUNNNING INDEX_GREEN
#define INDEX_DARKGREEN 1
#define INDEX_CONTROLLER_RUNNING INDEX_DARKGREEN
#define INDEX_LIGHTGREEN 2
#define INDEX_WORKER_RUNNING INDEX_LIGHTGREEN
#define INDEX_ORANGE 3
#define INDEX_UNKNOWN INDEX_ORANGE
#define INDEX_DARKORANGE 4
#define INDEX_CONTROLLER_UNKNOWN INDEX_DARKORANGE
#define INDEX_LIGHTORANGE 5
#define INDEX_WORKER_UNKNOWN INDEX_LIGHTORANGE
#define INDEX_RED 6
#define INDEX_STOPPED INDEX_RED
#define INDEX_DARKRED 7
#define INDEX_CONTROLLER_STOPPED INDEX_DARKRED
#define INDEX_LIGHTRED 8
#define INDEX_WORKER_STOPPED INDEX_LIGHTRED
#define INDEX_YELLOW 9
#define INDEX_PAUSED INDEX_YELLOW
#define INDEX_DARKYELLOW 10
#define INDEX_CONTROLLER_PAUSED INDEX_DARKYELLOW
#define INDEX_LIGHTYELLOW 11
#define INDEX_WORKER_PAUSED INDEX_LIGHTYELLOW

#endif

class statusLight_t : public QWidget
{
	Q_OBJECT

	public:
		typedef enum {	SERVICES_RUNNING,	CONTROLLER_RUNNING,		WORKER_RUNNING, 
						SERVICES_UNKNOWN,	CONTROLLER_UNKNOWN,		WORKER_UNKNOWN, 
						SERVICES_STOPPED,	CONTROLLER_STOPPED,		WORKER_STOPPED, 
						SERVICES_PAUSED,	CONTROLLER_PAUSED,		WORKER_PAUSED 
		} status_state_t;

	public slots:
		void setState(status_state_t);

	private:
		typedef struct { 
				QWidget*					light;
				QLabel*						label;
		} lights_t;

		typedef std::shared_ptr< lights_t > lights_ptr_t;

		std::array< lights_ptr_t, LIGHT_SIZE >	m_lights;
		QWidget*								m_light;
		QLabel*									m_label;
		QHBoxLayout*							m_layout;
		std::mutex								m_mutex;
		QString									m_staticLabel;

	protected:
		void setGlow(QWidget&);
		void setLightAndLabel(status_state_t);
		void initLightsAndLabels(void);

	public:

		statusLight_t(QWidget* p = nullptr, status_state_t s = status_state_t::SERVICES_UNKNOWN);
		~statusLight_t(void);

};

#endif // STATUSLIGHT_H
