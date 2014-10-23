#ifndef UTAB_H
#define UTAB_H

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

#include "namespacedialog.h"
#include "userdialog.h"

typedef QVector< userInfo_t > userVec_t;
typedef QMap< QString, userVec_t > userMap_t;

class userTab_t : public QWidget
{
	Q_OBJECT

	protected slots:
		void newNamespace(QString&);
		void editNamespace(QString&, QString&);
		void newUser(userInfo_t&);
		
		void dblClick(QTreeWidgetItem *, int);
		void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
		void onSelectionChanged(void);
		void onItemExpanded(QTreeWidgetItem*);

		void searchItems(void);
		void addChildItem(void);
		void addNamespaceItem(void);
		void removeItem(void);

	private:
		std::mutex			m_mutex;
		QVBoxLayout*		m_layout;
		QHBoxLayout*		m_hlayout;
		QFrame*				m_frame;
		QSpacerItem*		m_spacer;
		QPushButton*		m_searchItems;
		QPushButton*		m_addNamespace;
		QPushButton*		m_addItem;
		QPushButton*		m_removeItem;
		QTreeWidget*		m_tree;
		QTreeWidgetItem*	m_top;
		QSettings*			m_settings;

	protected:
		bool initializeNameSpacesAndUsers(void);
		void initializeButtons(void);

		//bool addUser(userInfo_t&);
		bool removeUser(QString&, QString&);
		bool getOrAddUserItem(QTreeWidgetItem&, QString&, QTreeWidgetItem&);
		bool userExists(QString&, QString&, bool warn = true);
		void updateUser(QTreeWidgetItem&, userInfo_t&);
		void editUser(userInfo_t&, userInfo_t&);

		bool addNamespace(QString&);
		bool removeNamespace(QString&);
		bool namespaceExists(QString&, bool warn = true);

		bool getUser(QString&, QString&, userInfo_t&);
		bool getAllUsers(userMap_t&);
		bool getUsersForNamespace(QString&, userVec_t&);
		bool getOrAddNamespace(QString&, QTreeWidgetItem&);
		
		QTreeWidgetItem* findChild(QTreeWidgetItem&, QString&);

		QString makeAddressString(userInfo_t&);

		

		//QString toFullKeyPath(QTreeWidgetItem*);
		/*void initEditable(void);
		void initButtons(void);
		void updateSettings(QTreeWidgetItem*, const QString&);
		void addGroupItems(QTreeWidgetItem*, const QStringList&);

		void addUsers(QString&, QString&);
		void addUser(QString&, QString&, QString&);*/
	public:
		userTab_t(QWidget* p = nullptr);
		~userTab_t(void);
};

#endif // UTAB_H
