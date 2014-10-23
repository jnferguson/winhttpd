#include "hosttab.h"

hostTab_t::hostTab_t(QWidget* p) : QWidget(p)
{
	QStringList		labels({"Domain", "Key", "Value"});

	m_layout		= new QVBoxLayout(this);
	m_tree			= new QTreeWidget(this);
	m_frame			= new QFrame(this);
	m_hlayout		= new QHBoxLayout(m_frame);
	m_searchItems	= new QPushButton(m_frame);
	m_addItem		= new QPushButton(m_frame);
	m_removeItem	= new QPushButton(m_frame);
	m_addSubItem	= new QPushButton(m_frame);
	m_top			= new QTreeWidgetItem(m_tree);
	m_spacer		= new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
	m_settings		= new QSettings(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

	m_settings->beginGroup("Plugins/VirtualHostManager/Options/Domains");
	//initializeButtons();

	m_frame->setFrameShape(QFrame::Shape::NoFrame);
	m_frame->setFrameShadow(QFrame::Shadow::Plain);

	m_hlayout->addItem(m_spacer);
	m_hlayout->addWidget(m_searchItems);
	m_hlayout->addWidget(m_removeItem);
	m_hlayout->addWidget(m_addItem);
	m_hlayout->addWidget(m_addSubItem);

	m_tree->setSortingEnabled(true);
	m_tree->header()->setHighlightSections(true);
	m_tree->header()->setProperty("showSortIndicator", QVariant(true));
	m_tree->setColumnCount(labels.size());
	m_tree->setHeaderLabels(labels);
	m_tree->setAlternatingRowColors(true);

	m_top->setText(0, "Hosts");

	initializeButtons();
	addKeys();

	/*if (false == initializeNameSpacesAndUsers()) {
		QMessageBox::warning(this, "Error", "Error while initializing the user list", QMessageBox::Ok);
		throw std::runtime_error("Error while initializing the user list");
	}*/

	m_layout->addWidget(m_frame);
	m_layout->addWidget(m_tree);


	connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &hostTab_t::dblClick);
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &hostTab_t::onSelectionChanged);
	connect(m_tree, &QTreeWidget::currentItemChanged, this, &hostTab_t::itemChanged);
	connect(m_tree, &QTreeWidget::itemExpanded, this, &hostTab_t::onItemExpanded);
	m_tree->resizeColumnToContents(0);
	return;

	return;
}

hostTab_t::~hostTab_t(void)
{
	return;
}

void
hostTab_t::initializeButtons(void)
{
	QPixmap	search(":/icons/Resources/search.png");
	QPixmap	plus(":/icons/Resources/plus.png");
	QPixmap	downplus(":/icons/Resources/downplus.png");
	QPixmap	minus(":/icons/Resources/minus.png");

	m_searchItems->setIcon(QIcon(search));
	m_searchItems->setDisabled(false);
	m_searchItems->setToolTip("Search Hosts");
	m_searchItems->setWhatsThis("Search Hosts");
	m_searchItems->setShortcut(QKeySequence("Ctrl+F"));
	//m_searchItems->setTabOrder()

	m_addItem->setIcon(QIcon(plus));
	m_addItem->setDisabled(false);
	m_addItem->setToolTip("Add New Host Key-Value Pair");
	m_addItem->setWhatsThis("Add New Host Key-Value Pair");
	m_addItem->setShortcut(QKeySequence("Ctrl+N"));

	m_addSubItem->setIcon(QIcon(downplus));
	m_addSubItem->setDisabled(false);
	m_addSubItem->setToolTip("Add New Host");
	m_addSubItem->setWhatsThis("Add New Host");
	m_addSubItem->setShortcut(QKeySequence("Ctrl+G"));

	m_removeItem->setIcon(QIcon(minus));
	m_removeItem->setDisabled(false);
	m_removeItem->setToolTip("Remove Host or Key-Value Pair");
	m_removeItem->setWhatsThis("Remove Host or Key-Value Pair");
	m_removeItem->setShortcut(QKeySequence("Ctrl+R"));

	connect(m_searchItems, &QPushButton::clicked, this, &hostTab_t::searchItems);
	connect(m_addItem, &QPushButton::clicked, this, &hostTab_t::addChildItem);
	connect(m_addSubItem, &QPushButton::clicked, this, &hostTab_t::addParentItem);
	connect(m_removeItem, &QPushButton::clicked, this, &hostTab_t::removeItem);

}


QStringList
hostTab_t::getFileSystem(QString& host)
{
	QStringList files; 
	QStringList xlate;
	QStringList fs; 

	m_settings->beginGroup("/" + host + "/FileSystem/RootDirectory");
	files = m_settings->childKeys();

	for (auto& f : files) {
		m_settings->beginGroup("/" + f);
		xlate = m_settings->allKeys();
		m_settings->endGroup();

		if (m_settings->value(f, "").toString().isEmpty()) 
			fs.push_back(f);

		for (auto& x : xlate) {	
			if (m_settings->value(f + "/" + x, "").toString().isEmpty())
				fs.push_back(f + "/" + x);
								
		}
		
		xlate.clear();
		
	}

	m_settings->endGroup();

	return fs;
}

void
hostTab_t::addKeys(void)
{
	QStringList	hosts(m_settings->childKeys());

	for (auto& h : hosts) {
		QTreeWidgetItem*	hptr(new QTreeWidgetItem(QStringList(h)));
		QStringList			keys("");

		m_top->addChild(hptr);

		m_settings->beginGroup("/" + h);
		keys = m_settings->allKeys();
		m_settings->endGroup();

		for (auto& k : keys) {
			if (k.contains("FileSystem", Qt::CaseInsensitive))
				continue;

			addKey(h, k);
		}

		keys = getFileSystem(h);

		for (auto& p : keys) 
			addFileSystemKey(h, p);
	}

	return;

}

QTreeWidgetItem*
hostTab_t::findChild(QTreeWidgetItem* i, QString& n)
{
	int cnt(0);

	if (nullptr == i)
		return nullptr;

	for (auto idx = 0; idx < i->childCount(); idx++) {
		QTreeWidgetItem* c = i->child(idx);

		if (nullptr == c)
			return nullptr;

		if (!c->data(0, 0).toString().compare(n, Qt::CaseInsensitive))
			return c;
	}

	return nullptr;

}

void
hostTab_t::addFileSystemKey(QString& host, QString& key)
{
	QStringList			parts(key.split("/", QString::SplitBehavior::SkipEmptyParts));
	QTreeWidgetItem*	ptr(findChild(m_top, host));
	QTreeWidgetItem*	fs(nullptr);
	QString				fp("");

	if (nullptr == ptr) {
		QMessageBox::warning(this, "Error while adding key", "Error while attempting to retrieve the domain '" + host + "'");
		return;
	}

	fs = findChild(ptr, QString("FileSystem"));

	if (nullptr == fs) {
		fs = new QTreeWidgetItem(QStringList("FileSystem"));
		ptr->addChild(fs);
		
	}

	ptr = fs;
	fp = "/"; 

	foreach(QString part, parts) {
		QTreeWidgetItem* tmp(nullptr);

		/*if (!parts.last().compare(part, Qt::CaseSensitive)) {
			tmp = new QTreeWidgetItem(QStringList(parts.last()));
			ptr->addChild(tmp);
			break;
		}*/

		tmp = findChild(ptr, part);

		if (nullptr == tmp) {

			fp += "/" + part;
			tmp = new QTreeWidgetItem(QStringList(fp));

			if (nullptr == ptr)
				break;

			ptr->addChild(tmp);
		} else
			ptr = tmp;
	}


	return;
}

void 
hostTab_t::addKey(QString& host, QString& key)
{
	QStringList			parts(key.split("/", QString::SplitBehavior::SkipEmptyParts));
	QTreeWidgetItem*	ptr(findChild(m_top, host));

	if (nullptr == ptr) {
		QMessageBox::warning(this, "Error while adding key", "Error while attempting to retrieve the domain '" + host + "'");
		return;
	}

	if (m_settings->value(host + "/" + key, "").toString().isEmpty())
		return;

	foreach(QString part, parts) {
		QTreeWidgetItem* tmp(nullptr);

		if (! parts.last().compare(part, Qt::CaseSensitive)) {

			tmp = new QTreeWidgetItem(QStringList(""));

			ptr->addChild(tmp);
			tmp->setData(0, Qt::DisplayRole, "");
			tmp->setData(1, Qt::DisplayRole, part);
			tmp->setData(2, Qt::DisplayRole, m_settings->value(host + "/" + key, "").toString());
			break;
		}

		tmp = findChild(ptr, part);

		if (nullptr == tmp) {
			tmp = new QTreeWidgetItem(QStringList(part));

			if (nullptr == ptr)
				break;

			ptr->addChild(tmp);
		}

		ptr = tmp;
	}

}

void
hostTab_t::addKey(QString& key)
{
	QStringList			parts(key.split("/", QString::SplitBehavior::SkipEmptyParts));
	QString				host(parts.first());
	QTreeWidgetItem*	ptr(m_top);

	if (nullptr == ptr)
		return;

	foreach(QString part, parts) {
		QTreeWidgetItem* tmp(nullptr);

		if (!parts.last().compare(part, Qt::CaseSensitive)) {
			tmp = new QTreeWidgetItem(QStringList("")); // part));
			ptr->addChild(tmp);
			tmp->setData(1, Qt::DisplayRole, part); //m_settings->value(key, ""));
			tmp->setData(2, Qt::DisplayRole, m_settings->value(key, ""));
			break;
		}

		tmp = findChild(ptr, part);

		if (nullptr == tmp) {
			tmp = new QTreeWidgetItem(QStringList(part));

			if (nullptr == ptr)
				break;

			ptr->addChild(tmp);
		}

		ptr = tmp;
	}

	return;
}

void
hostTab_t::dblClick(QTreeWidgetItem * i, int c)
{
	nameValueWidget_t*	n = nullptr;

	if (nullptr == i || i->childCount() || i == m_top)
		return;

	n = new nameValueWidget_t(this, false, i);
	n->set_name(i->data(0, Qt::DisplayRole).toString());
	n->set_value(i->data(1, Qt::DisplayRole).toString());

	connect(n, &nameValueWidget_t::setValuesWithItem, this, &hostTab_t::editItem); // addTLD
	n->exec();

	return;
}

void
hostTab_t::onItemExpanded(QTreeWidgetItem*)
{
	m_tree->resizeColumnToContents(0);
	return;
}

void
hostTab_t::itemChanged(QTreeWidgetItem* n, QTreeWidgetItem* p)
{
	m_tree->resizeColumnToContents(0);
	return;
}

void
hostTab_t::itemContentChanged(QTreeWidgetItem* i, int c)
{
	return;
}

void
hostTab_t::onSelectionChanged(void)
{
	m_tree->resizeColumnToContents(0);
	return;
}

void
hostTab_t::searchItems(void)
{
	QMessageBox::information(this, "title", "searchItems", QMessageBox::Ok);
	return;
}

void
hostTab_t::addChildItem(void)
{
	QList< QTreeWidgetItem* >	l = m_tree->selectedItems();
	nameValueWidget_t*			n = nullptr;

	if (0 == l.size()) {
		n = new nameValueWidget_t(this, true, m_top);
		connect(n, &nameValueWidget_t::setValuesWithItem, this, &hostTab_t::insertItem); // addTLD
		n->exec();
		return;
	}

	foreach(QTreeWidgetItem* i, l) {
		n = new nameValueWidget_t(this, false, i);
		connect(n, &nameValueWidget_t::setValuesWithItem, this, &hostTab_t::insertItem);
		n->exec();
	}

	return;
}

void
hostTab_t::addParentItem(void)
{
	QList< QTreeWidgetItem* >	l = m_tree->selectedItems();
	QTreeWidgetItem*			p = nullptr;
	nameValueWidget_t*			n = nullptr;

	if (0 == l.size()) {
		n = new nameValueWidget_t(this, true, m_top);
		connect(n, &nameValueWidget_t::setValuesWithItem, this, &hostTab_t::insertItem); // add TLD
		n->exec();
		return;
	}

	p = l.at(0);

	if (nullptr == p)
		p = m_top;

	n = new nameValueWidget_t(this, true, p);
	connect(n, &nameValueWidget_t::setValuesWithItem, this, &hostTab_t::insertItem);
	n->exec();
	return;

	/*foreach(QTreeWidgetItem* i, l) {
	QTreeWidgetItem* p = i->parent();
	n = new nameValueWidget_t(this, true, (nullptr == p ? m_top : p));
	connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::insertItem);
	n->exec();
	}*/

	return;
}

void
hostTab_t::insertItem(QTreeWidgetItem* i, const QString& n, const QString& v)
{
	QTreeWidgetItem* in = new QTreeWidgetItem(QStringList(n));
	QTreeWidgetItem* ptr = nullptr;
	QString p;

	in->setData(1, Qt::EditRole, QVariant(v));

	if (m_top == i)
		m_top->addChild(in);
	else
		i->addChild(in);

	in->parent()->setExpanded(true);

	updateSettings(in, v);
	return;
}

void
hostTab_t::editItem(QTreeWidgetItem* i, const QString& n, const QString& v)
{

	if (nullptr == i)
		return;

	i->setData(0, Qt::DisplayRole, QVariant(n));
	i->setData(1, Qt::DisplayRole, QVariant(v));

	updateSettings(i, v);
	return;
}

void
hostTab_t::updateSettings(QTreeWidgetItem* i, const QString& v)
{
	QTreeWidgetItem*	ptr = nullptr;
	QString				p = "";

	if (nullptr == i)
		return;

	for (ptr = i; ptr != m_top; ptr = ptr->parent())
		p.prepend(ptr->data(0, Qt::DisplayRole).toString() + "/");

	p.chop(1);
	m_settings->setValue(p, v);
	m_tree->resizeColumnToContents(0);
	return;
}

void
hostTab_t::removeItem(void)
{
	QList< QTreeWidgetItem* >	l = m_tree->selectedItems();

	if (l.isEmpty())
		return;

	foreach(QTreeWidgetItem* i, l) {
		if (m_top == i)
			continue;

		if (nullptr == i)
			continue;

		m_settings->remove(toFullKeyPath(i));
		delete i;
	}

	return;
}

QString
hostTab_t::toFullKeyPath(QTreeWidgetItem* item)
{
	QString str("");

	if (nullptr == item)
		throw std::runtime_error("configTab_t::toFullKeyPath(): Passed nullptr for parameter");

	//str = "/" + item->data(0, 0).toString();

	for (auto ptr = item; ptr != m_top; ptr = ptr->parent())
		str.prepend(ptr->data(0, 0).toString() + "/");

	str.chop(1);
	return str;
}
