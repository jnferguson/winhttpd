#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QSpacerItem>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QWidget>
#include <QStringList>
#include <QList>
#include <QSettings>

#include <QMessageBox>
#include "namevaluewidget.h"

class configTab_t : public QWidget
{
	Q_OBJECT

	private:
		QVBoxLayout*		m_layout;
		QHBoxLayout*		m_hlayout;
		QFrame*				m_frame;
		QSpacerItem*		m_spacer;
		QPushButton*		m_searchItems;
		QPushButton*		m_addSubItem;
		QPushButton*		m_addItem;
		QPushButton*		m_removeItem;
		QTreeWidget*		m_tree; 
		QTreeWidgetItem*	m_top;
		QSettings*			m_settings;

	protected:
		void initEditable(void);
		void initButtons(void);
		void updateSettings(QTreeWidgetItem*, const QString&);
		void addGroupItems(QTreeWidgetItem*, const QStringList&);

		void addKeys(void);
		void addKey(QString&);
		QTreeWidgetItem* findChild(QTreeWidgetItem*, QString&);
		QString toFullKeyPath(QTreeWidgetItem*);


	protected slots:
		void dblClick(QTreeWidgetItem *, int);
		void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
		void itemContentChanged(QTreeWidgetItem*, int);
		void onSelectionChanged(void);
		void onItemExpanded(QTreeWidgetItem*);
		
		void searchItems(void);
		void addChildItem(void);
		void insertItem(QTreeWidgetItem*, const QString&, const QString&);
		void editItem(QTreeWidgetItem*, const QString&, const QString&);
		void addParentItem(void);
		void removeItem(void);

	public:
		configTab_t(QWidget* p = nullptr);
		~configTab_t(void);

};

#endif // CONFIGTAB_H

