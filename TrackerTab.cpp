#include "TrackerTab.h"
#include "PieChartWidget.h"
#include "DataStore.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include <QRandomGenerator>
#include <QFrame>

TrackerTab::TrackerTab(QWidget *parent)
    : QWidget(parent)
{
    m_pie = new PieChartWidget(this);
    m_pie->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(12, 12, 12, 12);
    rightLayout->setSpacing(16);

    m_totalLabel = new QLabel("Total Spending: $0.00", rightPanel);
    QFont totalFont = m_totalLabel->font();
    totalFont.setPointSize(16);
    totalFont.setBold(true);
    m_totalLabel->setFont(totalFont);
    m_totalLabel->setAlignment(Qt::AlignCenter);

    rightLayout->addWidget(m_totalLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    rightLayout->addStretch();

    QFrame *entryFrame = new QFrame(rightPanel);
    entryFrame->setFrameShape(QFrame::StyledPanel);
    entryFrame->setFrameShadow(QFrame::Raised);
    QGridLayout *entryGrid = new QGridLayout(entryFrame);
    entryGrid->setSpacing(8);
    entryGrid->setContentsMargins(12, 12, 12, 12);

    QLabel *titleLbl  = new QLabel("Title:",  entryFrame);
    QLabel *amountLbl = new QLabel("Amount:", entryFrame);

    m_titleEdit  = new QLineEdit(entryFrame);
    m_titleEdit->setPlaceholderText("e.g. Coffee");

    m_amountEdit = new QLineEdit(entryFrame);
    m_amountEdit->setPlaceholderText("e.g. 4.50");

    m_addBtn = new QPushButton("Add Entry", entryFrame);
    m_addBtn->setFixedHeight(34);

    entryGrid->addWidget(titleLbl,     0, 0);
    entryGrid->addWidget(m_titleEdit,  0, 1);
    entryGrid->addWidget(amountLbl,    1, 0);
    entryGrid->addWidget(m_amountEdit, 1, 1);
    entryGrid->addWidget(m_addBtn,     2, 0, 1, 2);

    rightLayout->addWidget(entryFrame);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(m_pie,      3);
    mainLayout->addWidget(rightPanel, 1);

    connect(m_addBtn,     &QPushButton::clicked,     this, &TrackerTab::onAddEntry);
    connect(m_amountEdit, &QLineEdit::returnPressed,  this, &TrackerTab::onAddEntry);
    connect(m_titleEdit,  &QLineEdit::returnPressed,  this, &TrackerTab::onAddEntry);

    updateTotal();
}

void TrackerTab::resetPieChart()
{
    // Clear only the pie-chart entries; history entries stay in DataStore permanently.
    // We use a separate pieEntries list managed by GoalsTab signalling.
    // For simplicity: we do NOT clear DataStore entries here —
    // GoalsTab will call this when a new goal begins after a completed/failed goal.
    // The pie simply resets its visual; history tab keeps all entries forever.
    DataStore::instance().entries.clear();
    DataStore::instance().save();
    m_pie->refresh();
    updateTotal();
}

void TrackerTab::onAddEntry()
{
    QString title  = m_titleEdit->text().trimmed();
    QString amtStr = m_amountEdit->text().trimmed();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Missing title", "Please enter a title for this spending.");
        m_titleEdit->setFocus();
        return;
    }

    bool ok;
    double amount = amtStr.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Invalid amount", "Please enter a positive number for the amount.");
        m_amountEdit->setFocus();
        return;
    }

    SpendingEntry entry;
    entry.title    = title;
    entry.amount   = amount;
    entry.color    = randomColor();
    entry.dateTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    DataStore::instance().entries.append(entry);
    DataStore::instance().save();

    m_titleEdit->clear();
    m_amountEdit->clear();
    m_titleEdit->setFocus();

    m_pie->refresh();
    updateTotal();

    emit entryAdded();
}

void TrackerTab::updateTotal()
{
    double total = 0;
    for (const auto &e : DataStore::instance().entries)
        total += e.amount;
    m_totalLabel->setText(QString("Total Spending: $%1").arg(total, 0, 'f', 2));
}

QColor TrackerTab::randomColor()
{
    static double hue = QRandomGenerator::global()->generateDouble();
    hue += 0.618033988749895;
    if (hue >= 1.0) hue -= 1.0;
    return QColor::fromHsvF(hue, 0.70, 0.88);
}
