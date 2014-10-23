#include "namevaluewidget.h"

nameValueWidget_t::nameValueWidget_t(QWidget* p, bool d, QTreeWidgetItem* i) : QDialog(p), m_item(i), m_disable(d)
{
	m_layout	= new QVBoxLayout(this);
	m_name		= new QLineEdit(this);
	m_value		= new QLineEdit(this);
	m_buttons	= new QDialogButtonBox(this);

	resize(400, 100);

	if (true == m_disable) {
		m_layout->addWidget(m_name);
		
		m_name->setPlaceholderText("Name");

		delete m_value;
		m_value = nullptr;

	} else {
		m_layout->addWidget(m_name);
		m_layout->addWidget(m_value);

		m_name->setPlaceholderText("Name");
		m_value->setPlaceholderText("Value");
	}

	m_buttons->setOrientation(Qt::Horizontal);
	m_buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

	m_layout->addWidget(m_buttons);

	QObject::connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

	return;
}

nameValueWidget_t::~nameValueWidget_t(void)
{
	return;
}

void 
nameValueWidget_t::accept(void)
{

	if (nullptr != m_value && 0 == m_value->text().length()) {
		QMessageBox::warning(this, "Invalid Input", "A value parameter was not supplied. Please supply both a name and value", QMessageBox::Ok);
		return;
	}

	if (nullptr != m_item)
		emit setValuesWithItem(m_item, m_name->text(), (nullptr == m_value ? QString() : m_value->text()));
	else if (true == m_disable) 
		emit setValue(m_name->text());
	else 
		emit setValues(m_name->text(), m_value->text());

	QDialog::accept();
	this->deleteLater();

	return;
}

void 
nameValueWidget_t::reject(void)
{
	emit rejected();
	QDialog::reject();
	this->deleteLater();

	return;
}

void 
nameValueWidget_t::set_name(QString& n)
{
	m_name->setText(n);
	update();
	return;
}

void 
nameValueWidget_t::set_value(QString& v)
{
	m_value->setText(v);
	update();
	return;
}