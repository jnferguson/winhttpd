#include "namespacedialog.h"

namespaceDialog_t::namespaceDialog_t(QWidget* p) : QDialog(p), m_ns("")
{

	initWidget();
	setWindowTitle("Add New Namespace");
	m_group->setTitle("Add New Namespace");
	m_label->setText("Namespace");

	QObject::connect(m_buttons, &QDialogButtonBox::accepted, this, &namespaceDialog_t::accept);
	QObject::connect(m_buttons, &QDialogButtonBox::rejected, this, &namespaceDialog_t::reject);
	return;
}

namespaceDialog_t::namespaceDialog_t(QString& ns, QWidget* p) : QDialog(p), m_ns(ns)
{
	initWidget();
	setWindowTitle("Edit Namespace");
	m_group->setTitle("Edit Namespace");
	m_label->setText("Namespace");
	m_line->setText(ns);

	QObject::connect(m_buttons, &QDialogButtonBox::accepted, this, &namespaceDialog_t::acceptEdit);
	QObject::connect(m_buttons, &QDialogButtonBox::rejected, this, &namespaceDialog_t::reject);
}

namespaceDialog_t::~namespaceDialog_t()
{
	return;
}

void
namespaceDialog_t::initWidget(void)
{
	m_vlayout = new QVBoxLayout(this);
	m_group = new QGroupBox(this);
	m_buttons = new QDialogButtonBox(this);
	m_glayout = new QVBoxLayout(m_group);
	m_label = new QLabel(m_group);
	m_line = new QLineEdit(m_group);

	resize(400, 120);

	m_buttons->setOrientation(Qt::Horizontal);
	m_buttons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	m_glayout->addWidget(m_label);
	m_glayout->addWidget(m_line);
	m_vlayout->addWidget(m_group);
	m_vlayout->addWidget(m_buttons);
	return;
}

void 
namespaceDialog_t::accept(void)
{
	if (m_line->text().isEmpty()) {
		QMessageBox::information(this, "Error", "Error: no namespace was entered.");
		return;
	}

	emit newNamespace(m_line->text());
	QDialog::accept();
	deleteLater();
	return;
}

void
namespaceDialog_t::acceptEdit(void)
{
	if (m_line->text().isEmpty()) {
		QMessageBox::information(this, "Error", "Error: no namespace was entered.");
		return;
	}

	emit editNamespace(m_ns, m_line->text());
	QDialog::accept();
	deleteLater();
	return;
}

void 
namespaceDialog_t::reject(void)
{
	QDialog::reject();
	deleteLater();
	return;
}
