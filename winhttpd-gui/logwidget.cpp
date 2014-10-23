#include "logwidget.h"

#ifndef HAVE_LOG_TABLE_INDICES
#define HAVE_LOG_TABLE_INDICES
#define LOG_TABLE_ID_INDEX 0
#define LOG_TABLE_TYPE_INDEX 1
#define LOG_TABLE_CAT_INDEX 2
#define LOG_TABLE_TIMESTAMP_INDEX 3
#define LOG_TABLE_MESSAGE_INDEX 4
#define LOG_TABLE_SIZE 5
#endif

logWidget_t::logWidget_t(QWidget* p) : QWidget(p)
{
	QTableWidgetItem*	h[LOG_TABLE_SIZE]	= {nullptr};
	QThread*			t					= new QThread;

	m_layout	= new QVBoxLayout(this);
	m_table		= new QTableWidget(this);
	m_log		= new eventLog_t(nullptr); 
	
	connect(m_log, &eventLog_t::errorMessage, this, &logWidget_t::recvErrorMessage);

	m_clear		= new QAction("Clear All", m_table);
	m_remove	= new QAction("Remove All", m_table);

	m_clear->setToolTip("Clear all log entries from the current view");
	m_clear->setWhatsThis("Clear all log entries from the current view");
	m_clear->setShortcut(QKeySequence("Ctrl+L"));

	m_remove->setToolTip("Remove all log entries from the Windows event log (delete)");
	m_remove->setWhatsThis("Remove all log entries from the Windows event log (delete)");
	m_remove->setShortcut(QKeySequence("Ctrl+R"));


	/* 
		m_copyRow		= new QAction("Copy Entire Row", m_table);
		m_copyElement	= new QAction("Copy Individual Element", m_table);

	*/

	m_table->addAction(m_clear);
	m_table->addAction(m_remove);
	m_table->setContextMenuPolicy(Qt::ActionsContextMenu);

	connect(m_clear, &QAction::triggered, this, &logWidget_t::clearTable);
	connect(m_remove, &QAction::triggered, this, &logWidget_t::removeTable);

	m_log->moveToThread(t);

	m_table->setColumnCount(5);

	for (std::size_t idx = 0; idx < LOG_TABLE_SIZE; idx++) {
		h[idx] = new QTableWidgetItem();

		switch (idx) {
			case LOG_TABLE_ID_INDEX:
				h[idx]->setText("Record Number");
				m_table->hideColumn(0);
				break;
			case LOG_TABLE_TYPE_INDEX:
				h[idx]->setText("Type");
				break;
			case LOG_TABLE_CAT_INDEX:
				h[idx]->setText("Category");
				break;
			case LOG_TABLE_TIMESTAMP_INDEX:
				h[idx]->setText("Timestamp");
				break;
			case LOG_TABLE_MESSAGE_INDEX:
				h[idx]->setText("Message");
				break;
			default: // unpossible
				throw std::runtime_error("logWidget_t::logWidget_t(): The impossible logWidget_t() state occurred (this is a bug)");
				break;
		}

		m_table->setHorizontalHeaderItem(idx, h[idx]);
			
	}

	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setAlternatingRowColors(true);
	m_table->setSortingEnabled(true);
	m_table->horizontalHeader()->setStretchLastSection(true);
	m_table->verticalHeader()->setVisible(false);
	
	m_layout->addWidget(m_table);

	connect(m_log, &eventLog_t::entryReady, this, &logWidget_t::updateTable);
	connect(t, &QThread::started, m_log, &eventLog_t::start); 
	connect(t, &QThread::finished, t, &QThread::deleteLater);
	t->start();

	return;
}

logWidget_t::~logWidget_t()
{
	m_log->deleteLater();
	return;
}
  
void
logWidget_t::updateTable(eventLogEntry_t* e)
{
	std::lock_guard< std::mutex >		l(m_mutex);
	QTableWidgetItem*					items[LOG_TABLE_SIZE]	= {nullptr};
	int									rows					= m_table->rowCount();
	eventLogEntry_t*					p(nullptr);

	if (nullptr == e)
		return;

	m_table->setRowCount(rows+1);

	for (int idx = 0; idx < LOG_TABLE_SIZE; idx++){
		
		items[idx] = new QTableWidgetItem();

		switch (idx) {
			case LOG_TABLE_ID_INDEX:
				items[idx]->setData(Qt::DisplayRole, e->getId());
				m_table->setItem(rows, idx, items[idx]);
				break;
			case LOG_TABLE_TYPE_INDEX:
				items[idx]->setData(Qt::DisplayRole, e->getType());
				m_table->setItem(items[0]->row(), idx, items[idx]);
				break;
			case LOG_TABLE_CAT_INDEX:
				items[idx]->setData(Qt::DisplayRole, e->getCategory());
				m_table->setItem(items[0]->row(), idx, items[idx]);
				break;
			case LOG_TABLE_TIMESTAMP_INDEX:
				items[idx]->setData(Qt::DisplayRole, e->getTimeStamp());
				m_table->setItem(items[0]->row(), idx, items[idx]);
				break;
			case LOG_TABLE_MESSAGE_INDEX:
				items[idx]->setData(Qt::DisplayRole, e->getMessage());
				m_table->setItem(items[0]->row(), idx, items[idx]);
				break;
			default:
				throw std::runtime_error("logWidget_t::updateTable(): Unexpected and unknown event type encountered");
				break;	
		}

		
	}

	m_table->resizeColumnsToContents();
	m_table->horizontalHeader()->setStretchLastSection(true);
	m_table->update();
	delete e;
	return;
}

void
logWidget_t::recvErrorMessage(const QString& t, const QString& m, bool gle)
{
	util::error(t,m,gle);
	return;
}

void 
logWidget_t::clearTable(void)
{
	std::lock_guard< std::mutex >		l(m_mutex);

	m_table->setRowCount(0);
	return;
}

void
logWidget_t::removeTable(void)
{
	std::lock_guard< std::mutex >		l(m_mutex);

	m_table->setRowCount(0);
	m_log->clearEventLog();
	return;
}