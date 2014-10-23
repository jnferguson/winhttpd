#ifndef NAMEVALUEWIDGET_H
#define NAMEVALUEWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTreeWidgetItem>

#include <QMessageBox>

class nameValueWidget_t : public QDialog
{
	Q_OBJECT

	signals:
		void setValuesWithItem(QTreeWidgetItem*, const QString&, const QString&);
		void setValues(const QString&, const QString&);
		void setValue(const QString&);
		void rejected(void);

	private slots:
		virtual void accept(void);
		virtual void reject(void);

	private:
		QVBoxLayout*		m_layout;
		QLineEdit*			m_name;
		QLineEdit*			m_value;
		QDialogButtonBox*	m_buttons;
		QTreeWidgetItem*	m_item;
		bool				m_disable;

	protected:
	public:
		nameValueWidget_t(QWidget* p = nullptr, bool d = false, QTreeWidgetItem* i = nullptr);
		~nameValueWidget_t(void);
		void set_name(QString&);
		void set_value(QString&);
};

#endif // NAMEVALUEWIDGET_H
