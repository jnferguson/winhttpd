#include "statuslight.h"

statusLight_t::statusLight_t(QWidget* p, status_state_t s) : QWidget(p), m_staticLabel("The service is: "), m_light(nullptr), m_label(nullptr)
{
	std::lock_guard< std::mutex >	l(m_mutex);

	m_layout	= new QHBoxLayout(this);
	
	setParent(p);
	initLightsAndLabels();
	setLightAndLabel(s);
	return;
}

statusLight_t::~statusLight_t(void)
{
	std::lock_guard< std::mutex >	l(m_mutex);

	for (std::size_t idx = 0; idx < LIGHT_SIZE; idx++) {
		if (nullptr != m_lights.at(idx)) 
			m_lights.at(idx) = nullptr;
	}


	return;
}

void
statusLight_t::initLightsAndLabels(void)
{
	bool						l(m_mutex.try_lock());
	QString						s("border-width: 1px; border-style: inset; border-radius: 9px; ");

	for (std::size_t idx = 0; idx < LIGHT_SIZE; idx++) {
		lights_ptr_t tmp(new lights_t);

		tmp->light	= new QWidget(this);
		tmp->label	= new QLabel(this);

		tmp->light->setMaximumWidth(25);
		tmp->light->setMinimumWidth(25);
		tmp->light->setMaximumHeight(25);
		tmp->light->setMinimumHeight(25);

		tmp->label->setAlignment(Qt::AlignHCenter);

		switch (idx) {
			case INDEX_RUNNNING:
				tmp->label->setText(m_staticLabel + "Running & Functioning properly");
				tmp->light->setStyleSheet(s + "border-color: #00FF00; background: #00FF00");
				break;
			case INDEX_CONTROLLER_RUNNING:
				tmp->label->setText(m_staticLabel + "The Controller is running & Functioning properly");
				tmp->light->setStyleSheet(s + "border-color: #254117; background: #254117");
				break;
			case INDEX_WORKER_RUNNING:
				tmp->label->setText(m_staticLabel + "The Worker is running & Functioning properly");
				tmp->light->setStyleSheet(s + "border-color: #5FFB17; background: #5FFB17");
				break;
			case INDEX_UNKNOWN:
				tmp->label->setText(m_staticLabel + "In an unknown run-state");
				tmp->light->setStyleSheet(s + "border-color: #F88017; background: #F88017");
				break;
			case INDEX_CONTROLLER_UNKNOWN:
				tmp->label->setText(m_staticLabel + "The Controller is in an unknown run-state");
				tmp->light->setStyleSheet(s + "border-color: #CC6600; background: #CC6600");
				break;
			case INDEX_WORKER_UNKNOWN:
				tmp->label->setText(m_staticLabel + "The Worker is in an unknown run-state");
				tmp->light->setStyleSheet(s + "border-color: #E78A61; background: #E78A61");
				break;
			case INDEX_STOPPED:
				tmp->label->setText(m_staticLabel + "Is stopped & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #FF0000; background: #FF0000");
				break;
			case INDEX_CONTROLLER_STOPPED:
				tmp->label->setText(m_staticLabel + "The Controller is stopped & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #8C001A; background: #8C001A");
				break;
			case INDEX_WORKER_STOPPED:
				tmp->label->setText(m_staticLabel + "The Worker is stopped & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #F75D59; background: #F75D59");
				break;
			case INDEX_PAUSED:
				tmp->label->setText(m_staticLabel + "Is paused & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #FFFF00; background: #FFFF00");
				break;
			case INDEX_CONTROLLER_PAUSED:
				tmp->label->setText(m_staticLabel + "The Controller is paused & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #EAC117; background: #EAC117");
				break;
			case INDEX_WORKER_PAUSED:
				tmp->label->setText(m_staticLabel + "The Worker is paused & not handling requests");
				tmp->light->setStyleSheet(s + "border-color: #FFFFC2; background: #FFFFC2");
				break;
			default:
				throw std::runtime_error("statusLight_t::statusLight_t(): impossible status light index encountered");
				break;
		}

		tmp->label->setVisible(false);
		tmp->light->setVisible(false);


		m_lights.at(idx) = tmp;
	}

	if (true == l)
		m_mutex.unlock();

	return;
}

void 
statusLight_t::setGlow(QWidget& w)
{
	QGraphicsDropShadowEffect*	c(new QGraphicsDropShadowEffect(&w));

	c->setBlurRadius(30.0);
	c->setOffset(0.0,0.0);
	c->setColor(w.palette().color(QWidget::backgroundRole()));
	c->setEnabled(true);
	w.setGraphicsEffect(c);

	return;
}

void
statusLight_t::setLightAndLabel(status_state_t s)
{
	bool						l(m_mutex.try_lock());

	if (0 != m_layout->count()) {
		QGraphicsEffect* ge(nullptr);

		m_layout->removeWidget(m_light);
		m_layout->removeWidget(m_label);
		m_light->setVisible(false);
		m_label->setVisible(false);

		ge = m_light->graphicsEffect();

		if (nullptr != ge) 
			delete ge;

		m_light = nullptr;
		m_label = nullptr;
	}

	switch (s) {
		case status_state_t::SERVICES_RUNNING:
			m_light = m_lights.at(INDEX_RUNNNING)->light;
			m_label = m_lights.at(INDEX_RUNNNING)->label;
			break;
		case status_state_t::CONTROLLER_RUNNING:
			m_light = m_lights.at(INDEX_CONTROLLER_RUNNING)->light;
			m_label = m_lights.at(INDEX_CONTROLLER_RUNNING)->label;
			break;
		case status_state_t::WORKER_RUNNING:
			m_light = m_lights.at(INDEX_WORKER_RUNNING)->light;
			m_label = m_lights.at(INDEX_WORKER_RUNNING)->label;
			break;
		case status_state_t::SERVICES_UNKNOWN:
			m_light = m_lights.at(INDEX_UNKNOWN)->light;
			m_label = m_lights.at(INDEX_UNKNOWN)->label;
			break;
		case status_state_t::CONTROLLER_UNKNOWN:
			m_light = m_lights.at(INDEX_CONTROLLER_UNKNOWN)->light;
			m_label = m_lights.at(INDEX_CONTROLLER_UNKNOWN)->label;
			break;
		case status_state_t::WORKER_UNKNOWN:
			m_light = m_lights.at(INDEX_WORKER_UNKNOWN)->light;
			m_label = m_lights.at(INDEX_WORKER_UNKNOWN)->label;
			break;
		case status_state_t::SERVICES_STOPPED:
			m_light = m_lights.at(INDEX_STOPPED)->light;
			m_label = m_lights.at(INDEX_STOPPED)->label;
			break;
		case status_state_t::CONTROLLER_STOPPED:
			m_light = m_lights.at(INDEX_CONTROLLER_STOPPED)->light;
			m_label = m_lights.at(INDEX_CONTROLLER_STOPPED)->label;
			break;
		case status_state_t::WORKER_STOPPED:
			m_light = m_lights.at(INDEX_WORKER_STOPPED)->light;
			m_label = m_lights.at(INDEX_WORKER_STOPPED)->label;
			break;
		case status_state_t::SERVICES_PAUSED:
			m_light = m_lights.at(INDEX_PAUSED)->light;
			m_label = m_lights.at(INDEX_PAUSED)->label;
			break;
		case status_state_t::CONTROLLER_PAUSED:
			m_light = m_lights.at(INDEX_CONTROLLER_PAUSED)->light;
			m_label = m_lights.at(INDEX_CONTROLLER_PAUSED)->label;
			break;
		case status_state_t::WORKER_PAUSED:
			m_light = m_lights.at(INDEX_WORKER_PAUSED)->light;
			m_label = m_lights.at(INDEX_WORKER_PAUSED)->label;
			break;
		default:
			throw std::runtime_error("statusLight_t::statusLight_t(): impossible status light state specified");
			break;
	}

	m_light->setVisible(true);
	m_label->setVisible(true);
	
	setGlow(*m_light);

	//m_light->update();
	m_layout->addWidget(m_light);
	m_layout->addWidget(m_label);
	m_layout->setAlignment(Qt::AlignVCenter); 

	if (true == l)
		m_mutex.unlock();

	return;
}

void 
statusLight_t::setState(status_state_t s)
{
	setLightAndLabel(s);
	return;
}