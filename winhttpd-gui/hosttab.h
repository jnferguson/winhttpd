#ifndef VHOSTTAB_H
#define VHOSTTAB_H

#include <QWidget>
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

#include <mutex>

#include "namevaluewidget.h"

class hostTab_t : public QWidget
{
	Q_OBJECT

	private:
		std::mutex			m_mutex;
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
		void initializeButtons(void);
		void addKeys(void);
		QTreeWidgetItem* findChild(QTreeWidgetItem*, QString&);
		void addKey(QString&, QString&);
		void addKey(QString&);
		void dblClick(QTreeWidgetItem *, int);
		void onItemExpanded(QTreeWidgetItem*);
		void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
		void itemContentChanged(QTreeWidgetItem*, int);
		void onSelectionChanged(void);
		void searchItems(void);
		void addChildItem(void);
		void addParentItem(void);
		void insertItem(QTreeWidgetItem*, const QString&, const QString&);
		void editItem(QTreeWidgetItem*, const QString&, const QString&);
		void updateSettings(QTreeWidgetItem*, const QString&);
		void removeItem(void);
		QString toFullKeyPath(QTreeWidgetItem*);
		QStringList getFileSystem(QString&);
		void addFileSystemKey(QString&, QString&);

	public:
		hostTab_t(QWidget* p = nullptr);
		~hostTab_t(void);
};

#endif // VHOSTTAB_H
