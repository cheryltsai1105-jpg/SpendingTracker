#include "HistoryTab.h"
#include "DataStore.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSettings>
#include <QDateTime>
#include <QFont>

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    // ── Header ───────────────────────────────────────────────────────────────
    QLabel *header = new QLabel("Transaction History", this);
    QFont hf = header->font();
    hf.setPointSize(15);
    hf.setBold(true);
    header->setFont(hf);
    header->setAlignment(Qt::AlignCenter);
    lay->addWidget(header);

    // ── Table ─────────────────────────────────────────────────────────────────
    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels({"Title", "Amount ($)", "Date & Time"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(false);

    // Style: make it look clean
    m_table->setStyleSheet(
        "QTableWidget { gridline-color: #ddd; font-size: 11pt; }"
        "QHeaderView::section { background: #e3eaf5; font-weight: bold; "
        "                       padding: 6px; border: none; border-bottom: 1px solid #bbb; }"
        "QTableWidget::item { padding: 4px 8px; }"
        "QTableWidget::item:selected { background: #c5d8f5; color: black; }");

    lay->addWidget(m_table);

    loadFromSettings();
}

// ── Append the most recent DataStore entry ────────────────────────────────────
void HistoryTab::appendLatestEntry()
{
    const auto &entries = DataStore::instance().entries;
    if (entries.isEmpty()) return;

    const SpendingEntry &e = entries.last();

    // Parse ISO date to a friendlier display string
    QDateTime dt  = QDateTime::fromString(e.dateTime, Qt::ISODate);
    QString   dts = dt.isValid()
                  ? dt.toString("MMM dd, yyyy  hh:mm ap")
                  : e.dateTime;

    int row = m_table->rowCount();
    m_table->insertRow(row);

    auto *titleItem  = new QTableWidgetItem(e.title);
    auto *amountItem = new QTableWidgetItem(QString("$%1").arg(e.amount, 0, 'f', 2));
    auto *dateItem   = new QTableWidgetItem(dts);

    // Right-align amount
    amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Color the amount cell with a soft tint so it's easy to scan
    amountItem->setForeground(QColor(0x1a, 0x4f, 0x8a));

    m_table->setItem(row, 0, titleItem);
    m_table->setItem(row, 1, amountItem);
    m_table->setItem(row, 2, dateItem);

    m_table->scrollToBottom();

    saveAllToSettings();
}

// ── Persistence: separate "history" key, never cleared by goal resets ─────────
void HistoryTab::loadFromSettings()
{
    QSettings s("SpendingTrackerApp", "SpendingTracker");
    int n = s.beginReadArray("history");
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        QString title  = s.value("title").toString();
        double  amount = s.value("amount").toDouble();
        QString dtStr  = s.value("dateTime").toString();

        QDateTime dt  = QDateTime::fromString(dtStr, Qt::ISODate);
        QString   dts = dt.isValid()
                      ? dt.toString("MMM dd, yyyy  hh:mm ap")
                      : dtStr;

        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *ti = new QTableWidgetItem(title);
        auto *ai = new QTableWidgetItem(QString("$%1").arg(amount, 0, 'f', 2));
        auto *di = new QTableWidgetItem(dts);
        ai->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ai->setForeground(QColor(0x1a, 0x4f, 0x8a));

        m_table->setItem(row, 0, ti);
        m_table->setItem(row, 1, ai);
        m_table->setItem(row, 2, di);
    }
    s.endArray();
}

void HistoryTab::saveAllToSettings() const
{
    QSettings s("SpendingTrackerApp", "SpendingTracker");
    int rows = m_table->rowCount();
    s.beginWriteArray("history", rows);
    for (int i = 0; i < rows; ++i) {
        s.setArrayIndex(i);
        // Title is stored as-is; amount needs to be re-parsed from display text
        QString amtText = m_table->item(i, 1)->text(); // "$12.34"
        amtText.remove('$');
        s.setValue("title",    m_table->item(i, 0)->text());
        s.setValue("amount",   amtText.toDouble());
        // We store the display string directly as dateTime for history
        s.setValue("dateTime", m_table->item(i, 2)->text());
    }
    s.endArray();
}
