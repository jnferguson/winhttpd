#include "usertab.h"

/*
TODO
- fix mutex usage
- implement search functionality
- trace memory allocations; there are at least a few leaks
- implement drag and drop functionality
- implement functionality to allow namespace changes when modifying a user
- ??
*/

userTab_t::userTab_t(QWidget* p) : QWidget(p)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QStringList						labels({ "Namespace", "Password", "Real Name", 
											"Email Address", "Phone Number", "Address" 
									});

	m_layout		= new QVBoxLayout(this);
	m_tree			= new QTreeWidget(this);
	m_frame			= new QFrame(this);
	m_hlayout		= new QHBoxLayout(m_frame);
	m_searchItems	= new QPushButton(m_frame);
	m_addItem		= new QPushButton(m_frame);
	m_removeItem	= new QPushButton(m_frame);
	m_addNamespace	= new QPushButton(m_frame);
	m_top			= new QTreeWidgetItem(m_tree);
	m_spacer		= new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
	m_settings		= new QSettings(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

	initializeButtons();

	m_frame->setFrameShape(QFrame::Shape::NoFrame);
	m_frame->setFrameShadow(QFrame::Shadow::Plain);

	m_hlayout->addItem(m_spacer);
	m_hlayout->addWidget(m_searchItems);
	m_hlayout->addWidget(m_removeItem);
	m_hlayout->addWidget(m_addItem);
	m_hlayout->addWidget(m_addNamespace);

	m_tree->setSortingEnabled(true);
	m_tree->header()->setHighlightSections(true);
	m_tree->header()->setProperty("showSortIndicator", QVariant(true));
	m_tree->setColumnCount(labels.size());
	m_tree->setHeaderLabels(labels);
	m_tree->setAlternatingRowColors(true);

	m_top->setText(0, "User Configuration");

	if (false == initializeNameSpacesAndUsers()) {
		QMessageBox::warning(this, "Error", "Error while initializing the user list", QMessageBox::Ok);
		throw std::runtime_error("Error while initializing the user list");
	}

	m_layout->addWidget(m_frame);
	m_layout->addWidget(m_tree);


	connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &userTab_t::dblClick);
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &userTab_t::onSelectionChanged);
	connect(m_tree, &QTreeWidget::currentItemChanged, this, &userTab_t::itemChanged);
	connect(m_tree, &QTreeWidget::itemExpanded, this, &userTab_t::onItemExpanded);
	m_tree->resizeColumnToContents(0);
	return;
}

userTab_t::~userTab_t(void)
{
	return;
}

bool 
userTab_t::userExists(QString& ns, QString& user, bool warn)
{
	QString prefix("Plugins/UserManager/Options/Authentication/Namespaces/" + ns + "/Users");

	if (false == namespaceExists(ns, warn))
		return false;

	if (false == m_settings->contains(prefix + "/" + user)) {
		if (true == warn)
			QMessageBox::warning(this, "Error retrieving user record",
				"An unexpected error occurred while trying to obtain a user record: user '"
				+ ns + "\\\\" + user + "' does not exist.", QMessageBox::Ok);

		return false;
	}

	return true;
}

bool
userTab_t::getUser(QString& ns, QString& user, userInfo_t& ui)
{
	const QString			prefix("Plugins/UserManager/Options/Authentication/Namespaces/" + ns + "/Users");

	if (false == userExists(ns, user))
		return false;

	m_settings->beginGroup(prefix);
	ui.username = user;
	ui.password = m_settings->value("/" + user + "/Password", "").toString();
	ui.name = m_settings->value("/" + user + "/RealName", "").toString();
	ui.email = m_settings->value("/" + user + "/EmailAddress", "").toString();
	ui.phone = m_settings->value("/" + user + "/PhoneNumber", "").toString();
	ui.street = m_settings->value("/" + user + "/Street", "").toString();
	ui.city = m_settings->value("/" + user + "/City", "").toString();
	ui.state = m_settings->value("/" + user + "/State", "").toString();
	ui.country = m_settings->value("/" + user + "/Country", "").toString();
	ui.postal = m_settings->value("/" + user + "/PostalCode", "").toString();
	ui.algorithm = m_settings->value("/" + user + "/Algorithm", "").toString();
	ui.domain = ns; //m_settings->value("/" + user + "/Domain", "").toString();

	m_settings->endGroup();
	return true;
}

bool 
userTab_t::getAllUsers(userMap_t& uim)
{
	const QString					umprefix("Plugins/UserManager/Options/Authentication/Namespaces/");
	QStringList						ns;
	QStringList						users;
	userVec_t						uiv;
	userInfo_t						ui({ 0 });

	m_settings->beginGroup(umprefix);
	ns = m_settings->childKeys();
	m_settings->endGroup();

	for (auto& space : ns) {

		if (false == getUsersForNamespace(space, uiv))
			return false;

		uim.insert(space, uiv);
		uiv.clear();
	}

	return true;
}

bool
userTab_t::namespaceExists(QString& ns, bool warn)
{
	QString	prefix("Plugins/UserManager/Options/Authentication/Namespaces");

	if (false == m_settings->contains(prefix + "/" + ns)) {
		if (true == warn)
			QMessageBox::warning(this, "Error retrieving namespace record",
				"An unexpected error occurred while trying to obtain a namespace record: The namespace '"
				+ ns + "' does not exist.", QMessageBox::Ok);
		
		return false;
	}

	return true;
}

void
userTab_t::initializeButtons(void)
{
	QPixmap	search(":/icons/Resources/search.png");
	QPixmap	plus(":/icons/Resources/plus.png");
	QPixmap	minus(":/icons/Resources/minus.png");
	QPixmap	downplus(":/icons/Resources/downplus.png");

	m_searchItems->setIcon(QIcon(search));
	m_searchItems->setDisabled(false);
	m_searchItems->setToolTip("Search Users");
	m_searchItems->setWhatsThis("Search Users");
	m_searchItems->setShortcut(QKeySequence("Ctrl+F"));
	//m_searchItems->setTabOrder()

	m_addItem->setIcon(QIcon(plus));
	m_addItem->setDisabled(false);
	m_addItem->setToolTip("Add User");
	m_addItem->setWhatsThis("Add User");
	m_addItem->setShortcut(QKeySequence("Ctrl+N"));

	m_removeItem->setIcon(QIcon(minus));
	m_removeItem->setDisabled(false);
	m_removeItem->setToolTip("Remove User or Namespace");
	m_removeItem->setWhatsThis("Remove User or Namespace");
	m_removeItem->setShortcut(QKeySequence("Ctrl+R"));

	m_addNamespace->setIcon(QIcon(downplus));
	m_addNamespace->setDisabled(false);
	m_addNamespace->setToolTip("Add User");
	m_addNamespace->setWhatsThis("Add User");
	m_addNamespace->setShortcut(QKeySequence("Ctrl+G"));

	connect(m_searchItems, &QPushButton::clicked, this, &userTab_t::searchItems);
	connect(m_addItem, &QPushButton::clicked, this, &userTab_t::addChildItem);
	connect(m_removeItem, &QPushButton::clicked, this, &userTab_t::removeItem);
	connect(m_addNamespace, &QPushButton::clicked, this, &userTab_t::addNamespaceItem);

	return;
}

void 
userTab_t::searchItems(void)
{
	//std::lock_guard< std::mutex > l(m_mutex);

	return;
}

void 
userTab_t::addChildItem(void)
{
	//std::lock_guard< std::mutex > l(m_mutex);

	userDialog_t* u(new userDialog_t(this));
	connect(u, &userDialog_t::newInfo, this, &userTab_t::newUser);
	u->exec();
	return;
}

void 
userTab_t::addNamespaceItem(void)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	namespaceDialog_t*				n(nullptr);

	n = new namespaceDialog_t(this);
	connect(n, &namespaceDialog_t::newNamespace, this, &userTab_t::newNamespace);
	n->exec();
	return;
}

bool 
userTab_t::removeUser(QString& ns, QString& user)
{
	QMessageBox		mb;
	const QString	prefix("Plugins/UserManager/Options/Authentication/Namespaces/" + ns + "/Users/" + user);

	if (false == userExists(ns, user, false))
		return true;

	mb.setIcon(QMessageBox::Icon::Critical);
	mb.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
	mb.setText("You are about to delete a user!");
	mb.setInformativeText("Are you sure you wish to delete the user '"
						+ ns + "\\\\" + user + "'?\n\nThis action cannot be undone.");

	if (QMessageBox::StandardButton::No == mb.exec())
		return true;

	m_settings->remove(prefix);
	return true;
}

bool 
userTab_t::removeNamespace(QString& ns)
{
	QMessageBox		mb;
	const QString	prefix("Plugins/UserManager/Options/Authentication/Namespaces/" + ns);

	if (false == namespaceExists(ns, false))
		return true;

	mb.setIcon(QMessageBox::Icon::Critical);
	mb.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
	mb.setText("You are about to delete a namespace!");
	mb.setInformativeText("Are you sure you wish to delete the namespace '" + ns + "'?\n\nThis action cannot be undone.");

	if (QMessageBox::StandardButton::No == mb.exec())
		return true;

	m_settings->remove(prefix);
	return true;
}

void 
userTab_t::removeItem(void)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QList< QTreeWidgetItem* >		itemList = m_tree->selectedItems();
	QTreeWidgetItem*				item(nullptr);
	QTreeWidgetItem*				parent(nullptr);
	QString							ns("");
	QString							user("");

	if (itemList.isEmpty())
		return;
	
	if (1 != itemList.size()) {
		QMessageBox::warning(this, "Invalid state encountered", 
							"An unexpected and invalid state was encountered while removing the user record");
		return;
	}

	item = itemList.first();

	if (nullptr == item) {
		QMessageBox::warning(this, "Invalid state encountered", 
							"An unexpected and invalid state was encountered while removing the user record");
		return;
	}

	if (item == m_top)
		return;

	parent = item->parent();

	if (nullptr == parent) {
		QMessageBox::warning(this, "Invalid state encountered",
			"An unexpected and invalid state was encountered while removing the user record");
		return;
	}

	ns = parent->data(0, 0).toString();
	user = item->data(0, 0).toString();

	if (m_top == parent) {
		ns = item->data(0, 0).toString();
		removeNamespace(ns);
		delete item;
		return;
	}

	removeUser(ns, user);
	delete item;
	return;
}

void
userTab_t::editNamespace(QString& ons, QString& ns)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QTreeWidgetItem*				nsi(nullptr);
	QTreeWidgetItem*				onsi(nullptr);
	QTreeWidgetItem*				usi(nullptr);
	userVec_t						uv;

	if (false == namespaceExists(ons, false)) {
		if (true == namespaceExists(ns, false))
			return;

		newNamespace(ns);
		return;
	} 

	if (true == namespaceExists(ns, false)) {
		QMessageBox::warning(this, "Edit namespace error", 
			"You modified the namespace '" + ons + "' and renamed it to '" + ns + 
			"', however the namespace '" + ns + "' already exists.");

		return;
	}

	if (false == getUsersForNamespace(ons, uv)) {
		QMessageBox::warning(this, "Edit namespace error",
			"You modified the namespace '" + ons + "' and renamed it to '" + ns +
			"', however while processing this request we encountered "
			"an error accessing the namespaces users.");

		return;
	}

	onsi = findChild(*m_top, ons);

	if (nullptr == onsi) {
		QMessageBox::warning(this, "Edit namespace error",
			"You modified the namespace '" + ons + "' and renamed it to '" + ns +
			"', however while processing this request we encountered "
			"an error accessing the prior namespace.");

		return;
	}


	m_settings->beginGroup("Plugins/UserManager/Options/Authentication/Namespaces/");
	m_settings->setValue(ns, "");
	m_settings->setValue(ns + "/Users", "");
	m_settings->endGroup();

	nsi = new QTreeWidgetItem;
	(void)getOrAddNamespace(ns, *nsi);
	nsi->setData(0, 0, ns);

	// mutex deadlock
	// newNamespace(ns);
	//nsi = findChild(*m_top, ns);

	/*if (nullptr == nsi) {
		QMessageBox::warning(this, "Edit namespace error",
			"You modified the namespace '" + ons + "' and renamed it to '" + ns +
			"', however while processing this request we encountered "
			"an error accessing the newly created namespace");

		return;
	}*/

	for (auto& u : uv) {
		QTreeWidgetItem* tmp = findChild(*onsi, u.username);

		m_settings->remove("Plugins/UserManager/Options/Authentication/Namespaces/" + ons + "/Users/" + u.username);

		if (nullptr != tmp)
			delete tmp;
		
		u.domain = ns;
		usi = new QTreeWidgetItem;
		(void)getOrAddUserItem(*nsi, u.username, *usi);
		updateUser(*usi, u);
	}

	m_settings->remove("Plugins/UserManager/Options/Authentication/Namespaces/" + ons);
	delete onsi;


	return;
}

void
userTab_t::newNamespace(QString& ns)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QTreeWidgetItem*				nsi(nullptr);

	if (true == namespaceExists(ns, false)) {
		QMessageBox::warning(this, "Error Namespace already exists", "The requested namespace '" + ns + "' already exists");
		return;
	}

	m_settings->beginGroup("Plugins/UserManager/Options/Authentication/Namespaces/");
	m_settings->setValue(ns, "");
	m_settings->setValue(ns + "/Users", "");
	m_settings->endGroup();

	nsi = new QTreeWidgetItem;
	(void)getOrAddNamespace(ns, *nsi);
	nsi->setData(0, 0, ns);
	return;
}

void 
userTab_t::updateUser(QTreeWidgetItem& item, userInfo_t& info)
{

	item.setData(0, Qt::DisplayRole, info.username);
	item.setData(1, Qt::DisplayRole, info.password);
	item.setData(2, Qt::DisplayRole, info.name);
	item.setData(3, Qt::DisplayRole, info.email);
	item.setData(4, Qt::DisplayRole, info.phone);
	item.setData(5, Qt::DisplayRole, makeAddressString(info));

	m_settings->beginGroup("Plugins/UserManager/Options/Authentication/Namespaces/" + info.domain + "/Users");
	m_settings->setValue(info.username, "");
	m_settings->setValue(info.username + "/RealName", info.name);
	m_settings->setValue(info.username + "/Password", info.password);
	m_settings->setValue(info.username + "/EmailAddress", info.email);
	m_settings->setValue(info.username + "/PhoneNumber", info.phone);
	m_settings->setValue(info.username + "/Street", info.street);
	m_settings->setValue(info.username + "/City", info.city);
	m_settings->setValue(info.username + "/State", info.state);
	m_settings->setValue(info.username + "/Country", info.country);
	m_settings->setValue(info.username + "/PostalCode", info.postal);
	m_settings->setValue(info.username + "/Algorithm", info.algorithm);
	m_settings->endGroup();
	m_settings->sync();

	return;
}

void 
userTab_t::editUser(userInfo_t& ouser, userInfo_t& nuser)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QTreeWidgetItem*				nsi(nullptr);
	QTreeWidgetItem*				usi(nullptr);

	if (false == userExists(ouser.domain, ouser.username, false)) {
		QMessageBox::warning(this, "", "user does not exist, adding it: " + ouser.domain + "\\\\" + ouser.username);
		newUser(nuser);
		return;
	}

	nsi = findChild(*m_top, ouser.domain);

	if (nullptr == nsi) {
		QMessageBox::warning(this, "Error while modifying user!",
			"An error occured while attempting to modify a user, namely the user existed"
			" however the namespace it exists in somehow didn't. This is a bug.");
		return;
	}

	usi = findChild(*nsi, ouser.username);

	if (nullptr == usi) {
		QMessageBox::warning(this, "Error while modifying user!", 
			"An error occured while attempting to modify a user, namely the user existed"
			" however when we went to retrieve the record it somehow didn't. This is a bug.");
		return;
	}

	m_settings->remove("Plugins/UserManager/Options/Authentication/Namespaces/" + ouser.domain + "/Users/" + ouser.username);
	updateUser(*usi, nuser);
	return;
}

void
userTab_t::newUser(userInfo_t& ui)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QTreeWidgetItem*				nsi(nullptr);
	QTreeWidgetItem*				usi(nullptr);

	if (true == userExists(ui.domain, ui.username, false)) 
		return;
	
	nsi = findChild(*m_top, ui.domain);

	// this shouldnt be possible as whether the 
	// namespace exists or not is checked in userExists()
	if (nullptr == nsi) {
		QMessageBox::warning(this, "Error while adding user record", "An error that shouldn't be able to occur in fact did occur (report this as a bug please)");
		return;
	}

	usi = new QTreeWidgetItem;
	nsi->addChild(usi);
	updateUser(*usi, ui);

	return;
}

bool 
userTab_t::getUsersForNamespace(QString& ns, userVec_t& uiv)
{
	const QString	prefix("Plugins/UserManager/Options/Authentication/Namespaces/" + ns + "/Users");
	QStringList		users("");
	userInfo_t		ui({ 0 });

	if (false == namespaceExists(ns))
		return false;

	m_settings->beginGroup(prefix);
	users = m_settings->childKeys();
	m_settings->endGroup();

	for (auto& user : users) {
		if (false == getUser(ns, user, ui))
			return false;

		uiv.push_back(ui);
	}

	return true;
}

QTreeWidgetItem*
userTab_t::findChild(QTreeWidgetItem& parent, QString& name)
{
	for (auto idx = 0; idx < parent.childCount(); idx++) {
		QTreeWidgetItem* c = parent.child(idx);

		if (nullptr == c)
			continue;

		if (!c->data(0, 0).toString().compare(name, Qt::CaseInsensitive))
			return c;
	}

	return nullptr;
}

bool 
userTab_t::getOrAddNamespace(QString& ns, QTreeWidgetItem& nsi)
{
	QTreeWidgetItem* ret = findChild(*m_top, ns);

	if (nullptr == ret)
		m_top->addChild(&nsi);
	else // XXX JF FIXME - delete ns??
		nsi = *ret;

	return true;
}

bool 
userTab_t::getOrAddUserItem(QTreeWidgetItem& parent, QString& name, QTreeWidgetItem& usi)
{
	QTreeWidgetItem* ret = findChild(parent, name);

	if (nullptr == ret)
		parent.addChild(&usi);
	else
		usi = *ret;

	return true;
}

QString
userTab_t::makeAddressString(userInfo_t& ui)
{
	QString r("");

	if (!ui.street.isEmpty())
		r = ui.street + " ";
	if (!ui.city.isEmpty())
		r += ui.city + " ";
	if (!ui.state.isEmpty())
		r += ui.state + " ";
	if (!ui.country.isEmpty())
		r += ui.country + " ";
	if (!ui.postal.isEmpty())
		r += ui.postal;

	return r;
}

bool 
userTab_t::initializeNameSpacesAndUsers(void)
{
	userMap_t			uim;
	QTreeWidgetItem*	ns(nullptr);
	QTreeWidgetItem*	c(nullptr);

	if (false == getAllUsers(uim))
		return false;

	for (auto& itr = uim.begin(); itr != uim.end(); itr++) {
		ns = new QTreeWidgetItem;

		(void)getOrAddNamespace(const_cast<QString&>(itr.key()), *ns);

		ns->setData(0, Qt::DisplayRole, itr.key());

		for (auto& user : itr.value()) {
			c = new QTreeWidgetItem;

			(void)getOrAddUserItem(*ns, user.username, *c);

			c->setData(0, Qt::DisplayRole, user.username);
			c->setData(1, Qt::DisplayRole, user.password);
			c->setData(2, Qt::DisplayRole, user.name);
			c->setData(3, Qt::DisplayRole, user.email);
			c->setData(4, Qt::DisplayRole, user.phone);
			c->setData(5, Qt::DisplayRole, makeAddressString(user));
		}
	}

	return true;
}

void
userTab_t::onSelectionChanged(void)
{
	//std::lock_guard< std::mutex > l(m_mutex);

	m_tree->resizeColumnToContents(0);
	return;
}

void
userTab_t::itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
	//std::lock_guard< std::mutex > l(m_mutex);

	m_tree->resizeColumnToContents(0);
	return;
}

void
userTab_t::onItemExpanded(QTreeWidgetItem*)
{
	//std::lock_guard< std::mutex > l(m_mutex);

	m_tree->resizeColumnToContents(0);
	return;
}

void
userTab_t::dblClick(QTreeWidgetItem* i, int c)
{
	//std::lock_guard< std::mutex >	l(m_mutex);
	QTreeWidgetItem*				item(nullptr);
	QTreeWidgetItem*				parent(nullptr);
	QString							ns("");
	QString							user("");
	userVec_t						uvec;

	if (nullptr == i)
		return;

	parent = i->parent();

	if (nullptr == parent) {
		QMessageBox::warning(this, "Error", "Unexpected Error encountered with modifying user record.");
		return;
	}


	user = i->data(0, 0).toString();
	ns = parent->data(0, 0).toString();

	if (m_top == parent) {
		namespaceDialog_t* nsd = new namespaceDialog_t(i->data(0, 0).toString());
		connect(nsd, &namespaceDialog_t::editNamespace, this, &userTab_t::editNamespace);
		nsd->exec();
		return;
	}

	if (false == getUsersForNamespace(ns, uvec)) {
		QMessageBox::warning(this, "Error", "Unexpected Error encountered with user records for namespace.");
		return;
	}

	for (auto& u : uvec)
		if (!u.username.compare(user, Qt::CaseInsensitive)) {
			userDialog_t* ud = new userDialog_t(u, this);
			connect(ud, &userDialog_t::editUser, this, &userTab_t::editUser);
			ud->exec();
		}

	return;
}
