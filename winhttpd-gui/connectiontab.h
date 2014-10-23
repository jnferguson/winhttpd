#ifndef CONNECTIONTAB_H
#define CONNECTIONTAB_H

#include <QWidget>
#include <QTextBrowser>
#include <QHBoxLayout>

class connectionTab_t : public QWidget
{
	Q_OBJECT

	private:
		QHBoxLayout*	m_layout;
		QTextBrowser*	m_text;

	protected:
	public:
		connectionTab_t(QWidget* p = nullptr);
		~connectionTab_t(void);

};

#endif // CONNECTIONTAB_H
