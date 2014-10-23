#include "userdialog.h"

userDialog_t::userDialog_t(QWidget* p) : QDialog(p)
{
	QString type("Add User");

	initWidget(type);
	QObject::connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
	QObject::connect(m_show, &QCheckBox::stateChanged, this, &userDialog_t::checkboxChanged);

	return;
}

userDialog_t::userDialog_t(userInfo_t& ui, QWidget* p) : QDialog(p)
{
	QString type("Edit User");

	initWidget(type);
	signed int didx(-1);
	signed int aidx(-1);

	m_username->setText(ui.username);
	m_password->setText(ui.password);
	m_name->setText(ui.name);
	m_email->setText(ui.email);
	m_phone->setText(ui.phone);
	m_street->setText(ui.street);
	m_city->setText(ui.city);
	m_state->setText(ui.state);
	m_country->setText(ui.country);
	m_postal->setText(ui.postal);
	
	didx = m_domain->findText(ui.domain, Qt::MatchExactly);
	aidx = m_algorithm->findText(ui.algorithm, Qt::MatchExactly);

	if (-1 != didx)
		m_domain->setCurrentIndex(didx);
	if (-1 != aidx)
		m_algorithm->setCurrentIndex(aidx);

	m_domain->setDisabled(true);
	m_oldUser = ui;

	QObject::connect(m_buttons, SIGNAL(accepted()), this, SLOT(acceptEdit()));
	QObject::connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

	return;
}

userDialog_t::~userDialog_t(void)
{
	return;
}

void
userDialog_t::initWidget(QString& type)
{
	m_vlayout			= new QVBoxLayout(this);
	m_group				= new QGroupBox(this);
	m_buttons			= new QDialogButtonBox(this);
	m_glayout			= new QGridLayout(m_group);
	m_userLabel			= new QLabel(m_group);
	m_domainLabel		= new QLabel(m_group);
	m_username			= new QLineEdit(m_group);
	m_domain			= new QComboBox(m_group);
	m_passwordLabel		= new QLabel(m_group);
	m_algorithmLabel	= new QLabel(m_group);
	m_password			= new QLineEdit(m_group);
	m_algorithm			= new QComboBox(m_group);
	m_show				= new QCheckBox(m_group);
	m_realLabel			= new QLabel(m_group);
	m_phoneLabel		= new QLabel(m_group);
	m_name				= new QLineEdit(m_group);
	m_phone				= new QLineEdit(m_group);
	m_emailLabel		= new QLabel(m_group);
	m_streetLabel		= new QLabel(m_group);
	m_email				= new QLineEdit(m_group);
	m_street			= new QLineEdit(m_group);
	m_cityLabel			= new QLabel(m_group);
	m_stateLabel		= new QLabel(m_group);
	m_city				= new QLineEdit(m_group);
	m_state				= new QLineEdit(m_group);
	m_countryLabel		= new QLabel(m_group);
	m_postalLabel		= new QLabel(m_group);
	m_country			= new QLineEdit(m_group);
	m_postal			= new QLineEdit(m_group);

	resize(475, 345);

	m_glayout->addWidget(m_userLabel,			0, 0, 1, 1);
	m_glayout->addWidget(m_domainLabel,			0, 1, 1, 1);

	m_glayout->addWidget(m_username,			1, 0, 1, 1);
	m_glayout->addWidget(m_domain,				1, 1, 1, 1);
	
	m_glayout->addWidget(m_passwordLabel,		2, 0, 1, 1);
	m_glayout->addWidget(m_algorithmLabel,		2, 1, 1, 1);

	m_glayout->addWidget(m_password,			3, 0, 1, 1);
	m_glayout->addWidget(m_algorithm,			3, 1, 1, 1);
	m_glayout->addWidget(m_show,				3, 2, 1, 1);

	m_glayout->addWidget(m_realLabel,			4, 0, 1, 1);
	m_glayout->addWidget(m_phoneLabel,			4, 1, 1, 1);

	m_glayout->addWidget(m_name,				5, 0, 1, 1);
	m_glayout->addWidget(m_phone,				5, 1, 1, 2);

	m_glayout->addWidget(m_emailLabel,			6, 0, 1, 1);
	m_glayout->addWidget(m_streetLabel,			6, 1, 1, 1);

	m_glayout->addWidget(m_email,				7, 0, 1, 1);
	m_glayout->addWidget(m_street,				7, 1, 1, 2);

	m_glayout->addWidget(m_cityLabel,			8, 0, 1, 1);
	m_glayout->addWidget(m_stateLabel,			8, 1, 1, 1);

	m_glayout->addWidget(m_city,				9, 0, 1, 1);
	m_glayout->addWidget(m_state,				9, 1, 1, 2);

	m_glayout->addWidget(m_countryLabel,		10, 0, 1, 1);
	m_glayout->addWidget(m_postalLabel,			10, 1, 1, 1);

	m_glayout->addWidget(m_country,				11, 0, 1, 1);
	m_glayout->addWidget(m_postal,				11, 1, 1, 2);

	m_vlayout->addWidget(m_group);

	m_buttons->setOrientation(Qt::Horizontal);
	m_buttons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	m_vlayout->addWidget(m_buttons);

	this->setWindowTitle(type);
	m_group->setTitle(type);

	m_userLabel->setText("Username");
	m_domainLabel->setText("Domain");
	m_passwordLabel->setText("Password");
	m_algorithmLabel->setText("Algorithm");
	m_show->setText("Show Password?");
	m_realLabel->setText("Real Name");
	m_phoneLabel->setText("Phone Number");
	m_emailLabel->setText("Email Address");
	m_streetLabel->setText("Street Address");
	m_cityLabel->setText("City");
	m_stateLabel->setText("State / Province");
	m_countryLabel->setText("Country");
	m_postalLabel->setText("Postal Code");

	initComboBoxes();

	m_show->setChecked(true);
	m_password->setEchoMode(QLineEdit::EchoMode::Normal);

	return;
}

void
userDialog_t::initComboBoxes(void)
{
	QSettings s(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

	s.beginGroup("Plugins/UserManager/Options/Authentication/Namespaces");
	m_domainList = s.childKeys();
	s.endGroup();

	m_domainList.removeDuplicates();

	s.beginGroup("Plugins/UserManager/Options/Authentication/HashAlgorithms");
	m_algorithmsList = s.childKeys();
	s.endGroup();

	m_domain->clear();
	m_domain->insertItems(0, m_domainList);

	m_algorithm->clear();
	m_algorithm->insertItems(0, m_algorithmsList);

	return;
}

void 
userDialog_t::accept(void)
{
	userInfo_t ui;

	ui.username		= m_username->text();
	ui.password		= m_password->text();
	ui.name			= m_name->text();
	ui.phone		= m_phone->text();
	ui.email		= m_email->text();
	ui.street		= m_street->text();
	ui.city			= m_city->text();
	ui.state		= m_state->text();
	ui.country		= m_country->text();
	ui.postal		= m_postal->text();
	ui.domain		= m_domain->currentText();
	ui.algorithm	= m_algorithm->currentText();

	if (ui.username.isEmpty() || ui.password.isEmpty()) {
		QMessageBox::warning(this, "Error", "Adding a new user requires at least a username and password.", QMessageBox::Ok);
		return;
	}

	emit newInfo(ui); 
	QDialog::accept();
	deleteLater();
	return;
}

void 
userDialog_t::acceptEdit(void)
{
	userInfo_t ui;

	ui.username = m_username->text();
	ui.password = m_password->text();
	ui.name = m_name->text();
	ui.phone = m_phone->text();
	ui.email = m_email->text();
	ui.street = m_street->text();
	ui.city = m_city->text();
	ui.state = m_state->text();
	ui.country = m_country->text();
	ui.postal = m_postal->text();
	ui.domain = m_domain->currentText();
	ui.algorithm = m_algorithm->currentText();

	if (ui.username.isEmpty() || ui.password.isEmpty()) {
		QMessageBox::warning(this, "Error", "Adding a new user requires at least a username and password.", QMessageBox::Ok);
		return;
	}

	emit editUser(m_oldUser, ui);
	QDialog::accept();
	deleteLater();
	return;
}

void 
userDialog_t::reject(void)
{
	QDialog::reject();
	deleteLater();
	return;
}

void
userDialog_t::checkboxChanged(signed int state)
{
	QLineEdit::EchoMode mode(QLineEdit::EchoMode::Normal);

	switch (static_cast<Qt::CheckState>(state)) {
		case Qt::Unchecked:
			mode = QLineEdit::EchoMode::PasswordEchoOnEdit;
			break;

		default:
		case Qt::PartiallyChecked:
		case Qt::Checked:
			break;

	}

	m_password->setEchoMode(mode);
	return;
}