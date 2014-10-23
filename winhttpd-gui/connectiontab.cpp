#include "connectiontab.h"

connectionTab_t::connectionTab_t(QWidget* p) : QWidget(p)
{
	m_layout	= new QHBoxLayout(this);
	m_text		= new QTextBrowser(this);

	m_layout->addWidget(m_text);

	setParent(p);
	return;
}

connectionTab_t::~connectionTab_t()
{
	return;
}
