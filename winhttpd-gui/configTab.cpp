#include "configtab.h"

configTab_t::configTab_t(QWidget* p) : QWidget(p)
{
	QStringList					labels;

	m_layout		= new QVBoxLayout(this);
	m_tree			= new QTreeWidget(this);
	m_frame			= new QFrame(this);
	m_hlayout		= new QHBoxLayout(m_frame);
	m_searchItems	= new QPushButton(m_frame);
	m_addItem		= new QPushButton(m_frame);
	m_addSubItem	= new QPushButton(m_frame);
	m_removeItem	= new QPushButton(m_frame);
	m_top			= new QTreeWidgetItem(m_tree);
	m_spacer		= new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
	m_settings		= new QSettings(QSettings::Format::NativeFormat, QSettings::Scope::SystemScope, "NO!SOFT", "WINHTTPD");

	initButtons();

	m_frame->setFrameShape(QFrame::Shape::NoFrame);
	m_frame->setFrameShadow(QFrame::Shadow::Plain);

	m_hlayout->addItem(m_spacer);
	m_hlayout->addWidget(m_searchItems);
	m_hlayout->addWidget(m_removeItem);
	m_hlayout->addWidget(m_addItem);
	m_hlayout->addWidget(m_addSubItem);

	labels << "Name" << "Value";

	m_tree->setSortingEnabled(true);
	m_tree->header()->setHighlightSections(true);
	m_tree->header()->setProperty("showSortIndicator", QVariant(true));
	m_tree->setColumnCount(2);
	m_tree->setHeaderLabels(labels);
	m_tree->setAlternatingRowColors(true);

	m_top->setText(0, "Configuration");

	addKeys();

	m_layout->addWidget(m_frame);
	m_layout->addWidget(m_tree);


	connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &configTab_t::dblClick);
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &configTab_t::onSelectionChanged);
	connect(m_tree, &QTreeWidget::currentItemChanged, this, &configTab_t::itemChanged);
	connect(m_tree, &QTreeWidget::itemExpanded, this, &configTab_t::onItemExpanded);
	m_tree->resizeColumnToContents(0);
	return;
}

configTab_t::~configTab_t(void)
{
	return;
}

void
configTab_t::initButtons(void)
{
	QPixmap	search(":/icons/Resources/search.png");
	QPixmap	plus(":/icons/Resources/plus.png");
	QPixmap	downplus(":/icons/Resources/downplus.png");
	QPixmap	minus(":/icons/Resources/minus.png");

	m_searchItems->setIcon(QIcon(search));
	m_searchItems->setDisabled(false);
	m_searchItems->setToolTip("Search Configuration");
	m_searchItems->setWhatsThis("Search Configuration");
	m_searchItems->setShortcut(QKeySequence("Ctrl+F")); 
	//m_searchItems->setTabOrder()

	m_addItem->setIcon(QIcon(plus));
	m_addItem->setDisabled(false);
	m_addItem->setToolTip("Add Configuration Item");
	m_addItem->setWhatsThis("Add Configuration Item");
	m_addItem->setShortcut(QKeySequence("Ctrl+N"));

	m_addSubItem->setIcon(QIcon(downplus));
	m_addSubItem->setDisabled(false);
	m_addSubItem->setToolTip("Add Configuration Category");
	m_addSubItem->setWhatsThis("Add Configuration Category");
	m_addSubItem->setShortcut(QKeySequence("Ctrl+G"));

	m_removeItem->setIcon(QIcon(minus));
	m_removeItem->setDisabled(false);
	m_removeItem->setToolTip("Remove Configuration Item or Category");
	m_removeItem->setWhatsThis("Remove Configuration Item or Category");
	m_removeItem->setShortcut(QKeySequence("Ctrl+R"));

	connect(m_searchItems, &QPushButton::clicked, this, &configTab_t::searchItems);
	connect(m_addItem, &QPushButton::clicked, this, &configTab_t::addChildItem);
	connect(m_addSubItem, &QPushButton::clicked, this, &configTab_t::addParentItem);
	connect(m_removeItem, &QPushButton::clicked, this, &configTab_t::removeItem);

	return;
}
void 
configTab_t::addKeys(void)
{
	QStringList allkeys(m_settings->allKeys());

	foreach(QString key, allkeys)
		addKey(key);
		
	return;

}

QTreeWidgetItem* 
configTab_t::findChild(QTreeWidgetItem* i, QString& n)
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
configTab_t::addKey(QString& key)
{
	QStringList			parts(key.split("/", QString::SplitBehavior::SkipEmptyParts));
	QTreeWidgetItem*	ptr(m_top);

	if (nullptr == ptr) 
		return;

	foreach(QString part, parts) {
		QTreeWidgetItem* tmp(nullptr);
		
		if (!parts.last().compare(part, Qt::CaseSensitive)) {
			tmp = new QTreeWidgetItem(QStringList(part));
			ptr->addChild(tmp);
			tmp->setData(1, Qt::DisplayRole, m_settings->value(key, ""));
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
configTab_t::dblClick(QTreeWidgetItem * i, int c)
{
	nameValueWidget_t*	n = nullptr;

	if (nullptr == i || i->childCount() || i == m_top)
		return;

	n = new nameValueWidget_t(this, false, i);
	n->set_name(i->data(0, Qt::DisplayRole).toString());
	n->set_value(i->data(1, Qt::DisplayRole).toString());

	connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::editItem); // addTLD
	n->exec();

	return;
}

void
configTab_t::onItemExpanded(QTreeWidgetItem*)
{
	m_tree->resizeColumnToContents(0);
	return;
}
void 
configTab_t::itemChanged(QTreeWidgetItem* n, QTreeWidgetItem* p)
{
	m_tree->resizeColumnToContents(0);
	return;
}

void
configTab_t::itemContentChanged(QTreeWidgetItem* i, int c)
{
	return;
}

void
configTab_t::onSelectionChanged(void)
{
	m_tree->resizeColumnToContents(0);
	return;
}

void 
configTab_t::searchItems(void)
{
	QMessageBox::information(this, "title", "searchItems", QMessageBox::Ok);
	return;
}

void 
configTab_t::addChildItem(void)
{
	QList< QTreeWidgetItem* >	l = m_tree->selectedItems();
	nameValueWidget_t*			n = nullptr;

	if (0 == l.size()) {
		n = new nameValueWidget_t(this, true, m_top);
		connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::insertItem); // addTLD
		n->exec();
		return;
	}

	foreach(QTreeWidgetItem* i, l) {
			n = new nameValueWidget_t(this, false, i);
			connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::insertItem);
			n->exec();
	}

	return;
}

void 
configTab_t::addParentItem(void)
{
	QList< QTreeWidgetItem* >	l = m_tree->selectedItems();
	QTreeWidgetItem*			p = nullptr;
	nameValueWidget_t*			n = nullptr;

	if (0 == l.size()) {
		n = new nameValueWidget_t(this, true, m_top);
		connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::insertItem); // add TLD
		n->exec();
		return;
	}

	p = l.at(0);

	if (nullptr == p)
		p = m_top;

	n = new nameValueWidget_t(this, true, p);
	connect(n, &nameValueWidget_t::setValuesWithItem, this, &configTab_t::insertItem);
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
configTab_t::insertItem(QTreeWidgetItem* i, const QString& n, const QString& v)
{
	QTreeWidgetItem* in		= new QTreeWidgetItem(QStringList(n));
	QTreeWidgetItem* ptr	= nullptr;
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
configTab_t::editItem(QTreeWidgetItem* i, const QString& n, const QString& v)
{

	if (nullptr == i)
		return;

	i->setData(0, Qt::DisplayRole, QVariant(n));
	i->setData(1, Qt::DisplayRole, QVariant(v));

	updateSettings(i, v);
	return;
}

void 
configTab_t::updateSettings(QTreeWidgetItem* i, const QString& v)
{
	QTreeWidgetItem*	ptr	= nullptr;
	QString				p	= "";

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
configTab_t::removeItem(void)
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
configTab_t::toFullKeyPath(QTreeWidgetItem* item)
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