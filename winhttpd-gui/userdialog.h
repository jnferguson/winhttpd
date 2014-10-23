#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QMessageBox>

typedef struct _userInfo_t
{
	QString username;
	QString domain;
	QString password;
	QString algorithm;
	QString name;
	QString phone;
	QString email;
	QString street;
	QString city;
	QString state;
	QString country;
	QString postal;

	_userInfo_t(QString u = "", QString d = "", QString p = "",
		QString a = "", QString n = "", QString ph = "", QString e = "",
		QString s = "", QString c = "", QString st = "",
		QString co = "", QString po = "") : username(u), domain(d), password(p), algorithm(a),
		name(n), phone(ph), email(e), street(s), city(c),
		state(st), country(co), postal(po)
	{
		return;
	}

	~_userInfo_t(void) { return; }
} userInfo_t;

class userDialog_t : public QDialog
{
	Q_OBJECT
	
	signals:
		void newInfo(userInfo_t&);
		void editUser(userInfo_t&, userInfo_t&);

	private slots:
		virtual void checkboxChanged(signed int);
		virtual void accept(void);
		virtual void acceptEdit(void);
		virtual void reject(void);

	private:
		QStringList			m_domainList;
		QStringList			m_algorithmsList;
		userInfo_t			m_oldUser;

		QVBoxLayout*		m_vlayout;
		QGroupBox*			m_group;
		QGridLayout*		m_glayout;
		QLabel*				m_userLabel;
		QLabel*				m_domainLabel;
		QLineEdit*			m_username;
		QComboBox*			m_domain;
		QLabel*				m_passwordLabel;
		QLabel*				m_algorithmLabel;
		QLineEdit*			m_password;
		QComboBox*			m_algorithm;
		QCheckBox*			m_show;
		QLabel*				m_realLabel;
		QLabel*				m_phoneLabel;
		QLineEdit*			m_name;
		QLineEdit*			m_phone;
		QLabel*				m_emailLabel;
		QLabel*				m_streetLabel;
		QLineEdit*			m_email;
		QLineEdit*			m_street;
		QLabel*				m_cityLabel;
		QLabel*				m_stateLabel;
		QLineEdit*			m_city;
		QLineEdit*			m_state;
		QLabel*				m_countryLabel;
		QLabel*				m_postalLabel;
		QLineEdit*			m_country;
		QLineEdit*			m_postal;
		QDialogButtonBox*	m_buttons;

	protected:
		void initWidget(QString&);
		void initComboBoxes(void);
		//bool validate(void);

	public:
		userDialog_t(QWidget* p = nullptr);
		userDialog_t(userInfo_t& ui, QWidget* p = nullptr);

		~userDialog_t(void);



};



#endif // USERDIALOG_H
