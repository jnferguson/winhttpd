#ifndef NAMESPACEDIALOG_H
#define NAMESPACEDIALOG_H

#include <QtWidgets/QWidget>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QMessageBox>


class namespaceDialog_t : public QDialog
{
	Q_OBJECT

	signals:
		void newNamespace(QString&);
		void editNamespace(QString&, QString&);

	private slots:
		virtual void accept(void);
		virtual void acceptEdit(void);
		virtual void reject(void);

	private:
		QVBoxLayout*		m_glayout;
		QGroupBox*			m_group;
		QVBoxLayout*		m_vlayout;
		QLabel*				m_label;
		QLineEdit*			m_line;
		QDialogButtonBox*	m_buttons;
		QString				m_ns;

	protected:
		void initWidget(void);

	public:
		namespaceDialog_t(QWidget* p = nullptr);
		namespaceDialog_t(QString&, QWidget* p = nullptr);
		~namespaceDialog_t(void);

};


#endif // NAMESPACEDIALOG_H
