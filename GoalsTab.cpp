#include "GoalsTab.h"
#include "BarChartWidget.h"
#include "DataStore.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QDateTime>
#include <QDate>
#include <QMessageBox>
#include <QtMath>

// ── Helpers ───────────────────────────────────────────────────────────────────

// Return Monday of the week containing `date`
static QDate mondayOf(const QDate &date)
{
    int dow = date.dayOfWeek(); // 1=Mon … 7=Sun
    return date.addDays(1 - dow);
}

// Compute goal end QDateTime from goal start + range
static QDateTime computeEndDateTime(const QDateTime &start, int count, const QString &unit)
{
    QDateTime end = start;
    if      (unit == "day")   end = start.addDays(count);
    else if (unit == "week")  end = start.addDays(count * 7);
    else if (unit == "month") end = start.addMonths(count);
    else if (unit == "year")  end = start.addYears(count);
    return end;
}

// ── Constructor ───────────────────────────────────────────────────────────────
GoalsTab::GoalsTab(QWidget *parent)
    : QWidget(parent)
{
    // ── Left: bar chart ───────────────────────────────────────────────────────
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftVBox = new QVBoxLayout(leftPanel);
    leftVBox->setContentsMargins(8, 8, 8, 4);
    leftVBox->setSpacing(4);

    m_bar = new BarChartWidget(leftPanel);
    m_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_weekRangeLabel = new QLabel("", leftPanel);
    m_weekRangeLabel->setAlignment(Qt::AlignCenter);
    QFont wf = m_weekRangeLabel->font();
    wf.setPointSize(9);
    m_weekRangeLabel->setFont(wf);
    m_weekRangeLabel->setStyleSheet("color: #555;");

    leftVBox->addWidget(m_bar, 1);
    leftVBox->addWidget(m_weekRangeLabel);

    // ── Right: info + entry ───────────────────────────────────────────────────
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightVBox = new QVBoxLayout(rightPanel);
    rightVBox->setContentsMargins(12, 12, 12, 12);
    rightVBox->setSpacing(10);

    // Goal summary
    m_goalSummaryLabel = new QLabel("No goal set.", rightPanel);
    m_goalSummaryLabel->setAlignment(Qt::AlignCenter);
    m_goalSummaryLabel->setWordWrap(true);
    QFont gsf = m_goalSummaryLabel->font();
    gsf.setPointSize(13);
    gsf.setBold(true);
    m_goalSummaryLabel->setFont(gsf);
    m_goalSummaryLabel->setStyleSheet("color: #1a4f8a;");

    // Daily budget
    m_dailyBudgetLabel = new QLabel("", rightPanel);
    m_dailyBudgetLabel->setAlignment(Qt::AlignCenter);
    m_dailyBudgetLabel->setWordWrap(true);
    QFont dbf = m_dailyBudgetLabel->font();
    dbf.setPointSize(11);
    m_dailyBudgetLabel->setFont(dbf);
    m_dailyBudgetLabel->setStyleSheet("color: #2e7d32;");

    rightVBox->addWidget(m_goalSummaryLabel);
    rightVBox->addWidget(m_dailyBudgetLabel);
    rightVBox->addStretch();

    // ── Congrats frame (hidden normally) ─────────────────────────────────────
    m_congratsFrame = new QFrame(rightPanel);
    m_congratsFrame->setFrameShape(QFrame::StyledPanel);
    m_congratsFrame->setStyleSheet(
        "QFrame { background: #e8f5e9; border: 2px solid #43a047; border-radius: 8px; }");
    QVBoxLayout *cl = new QVBoxLayout(m_congratsFrame);
    auto *clbl = new QLabel("🎉 Goal achieved!\nGreat job staying on budget.", m_congratsFrame);
    clbl->setAlignment(Qt::AlignCenter);
    QFont cf = clbl->font(); cf.setPointSize(11); cf.setBold(true); clbl->setFont(cf);
    cl->addWidget(clbl);
    m_congratsFrame->hide();
    rightVBox->addWidget(m_congratsFrame);
    rightVBox->addStretch();

    // ── Entry box ─────────────────────────────────────────────────────────────
    m_entryFrame = new QFrame(rightPanel);
    m_entryFrame->setFrameShape(QFrame::StyledPanel);
    m_entryFrame->setFrameShadow(QFrame::Raised);
    QGridLayout *eg = new QGridLayout(m_entryFrame);
    eg->setContentsMargins(12, 12, 12, 12);
    eg->setSpacing(8);

    eg->addWidget(new QLabel("Spending goal ($):", m_entryFrame), 0, 0);
    m_goalAmountEdit = new QLineEdit(m_entryFrame);
    m_goalAmountEdit->setPlaceholderText("e.g. 500");
    eg->addWidget(m_goalAmountEdit, 0, 1);

    eg->addWidget(new QLabel("Time range:", m_entryFrame), 1, 0);
    QWidget *rangeRow = new QWidget(m_entryFrame);
    QHBoxLayout *rh = new QHBoxLayout(rangeRow);
    rh->setContentsMargins(0,0,0,0); rh->setSpacing(6);
    m_rangeCountEdit = new QLineEdit(rangeRow);
    m_rangeCountEdit->setPlaceholderText("1");
    m_rangeCountEdit->setFixedWidth(50);
    m_rangeUnitCombo = new QComboBox(rangeRow);
    m_rangeUnitCombo->addItems({"day", "week", "month", "year"});
    m_rangeUnitCombo->setCurrentIndex(1); // default: week
    rh->addWidget(m_rangeCountEdit);
    rh->addWidget(m_rangeUnitCombo);
    rh->addStretch();
    eg->addWidget(rangeRow, 1, 1);

    m_setGoalBtn = new QPushButton("Set Goal", m_entryFrame);
    m_setGoalBtn->setFixedHeight(34);
    eg->addWidget(m_setGoalBtn, 2, 0, 1, 2);

    rightVBox->addWidget(m_entryFrame);

    // ── Main split layout ─────────────────────────────────────────────────────
    QHBoxLayout *main = new QHBoxLayout(this);
    main->setContentsMargins(8, 8, 8, 8);
    main->setSpacing(12);
    main->addWidget(leftPanel,  3);
    main->addWidget(rightPanel, 1);

    // ── Congrats auto-dismiss timer ───────────────────────────────────────────
    m_congratsTimer = new QTimer(this);
    m_congratsTimer->setSingleShot(true);
    connect(m_congratsTimer, &QTimer::timeout, this, [this]() {
        m_congratsFrame->hide();
    });

    connect(m_setGoalBtn, &QPushButton::clicked, this, &GoalsTab::onSetGoal);

    // Restore state on launch
    advanceBarChartWeekIfNeeded();
    updateInfoPanel();

    // If a goal was active when the app closed, re-lock the entry box
    if (DataStore::instance().goal.active)
        lockEntryBox(true);
}

// ── Set goal ─────────────────────────────────────────────────────────────────
void GoalsTab::onSetGoal()
{
    bool ok1, ok2;
    double goalAmt = m_goalAmountEdit->text().trimmed().toDouble(&ok1);
    int    count   = m_rangeCountEdit->text().trimmed().toInt(&ok2);
    if (!ok2 || count <= 0) { count = 1; ok2 = true; }

    if (!ok1 || goalAmt <= 0) {
        QMessageBox::warning(this, "Invalid goal", "Please enter a positive spending goal amount.");
        return;
    }

    QString unit = m_rangeUnitCombo->currentText();

    GoalData &g  = DataStore::instance().goal;
    g.active     = true;
    g.totalGoal  = goalAmt;
    g.rangeCount = count;
    g.rangeUnit  = unit;

    // If entries already exist before the goal is set, backdate the goal start
    // to the earliest entry so all prior spending counts toward this goal.
    QDateTime goalStart = QDateTime::currentDateTime();
    const auto &entries = DataStore::instance().entries;
    if (!entries.isEmpty()) {
        QDateTime earliest = QDateTime::fromString(entries.first().dateTime, Qt::ISODate);
        for (const auto &e : entries) {
            QDateTime dt = QDateTime::fromString(e.dateTime, Qt::ISODate);
            if (dt.isValid() && dt < earliest)
                earliest = dt;
        }
        if (earliest.isValid() && earliest < goalStart)
            goalStart = earliest;
    }

    g.startDateTime = goalStart.toString(Qt::ISODate);
    g.endDateTime   = computeEndDateTime(goalStart, count, unit).toString(Qt::ISODate);
    DataStore::instance().save();

    lockEntryBox(true);
    advanceBarChartWeekIfNeeded();
    updateInfoPanel();
}

// ── Called from MainWindow whenever Tab 1 adds an entry ──────────────────────
void GoalsTab::onEntryAdded()
{
    advanceBarChartWeekIfNeeded();
    m_bar->refresh();
    updateInfoPanel();
    checkGoalStatus();
}

// ── Check goal completion / failure ──────────────────────────────────────────
void GoalsTab::checkGoalStatus()
{
    GoalData &g = DataStore::instance().goal;
    if (!g.active) return;

    QDateTime now   = QDateTime::currentDateTime();
    QDateTime end   = QDateTime::fromString(g.endDateTime, Qt::ISODate);
    double    spent = spentSinceGoalStart();

    // Over budget → fail
    if (spent > g.totalGoal) {
        showGoalFailed();
        return;
    }

    // Past deadline
    if (now >= end) {
        // Under or equal → success
        showCongrats();
    }
}

// ── Advance bar chart to the current week if we rolled over ──────────────────
void GoalsTab::advanceBarChartWeekIfNeeded()
{
    QDate today   = QDate::currentDate();
    QDate monday  = mondayOf(today);

    if (m_bar->weekStart() != monday) {
        m_bar->setWeekStart(monday);
        QDate sunday = monday.addDays(6);
        m_weekRangeLabel->setText(
            QString("Week: %1 – %2")
                .arg(monday.toString("MMM dd"))
                .arg(sunday.toString("MMM dd yyyy")));
    }
    m_bar->refresh();
}

// ── Update info panel labels ──────────────────────────────────────────────────
void GoalsTab::updateInfoPanel()
{
    GoalData &g = DataStore::instance().goal;
    if (!g.active || g.totalGoal <= 0) {
        m_goalSummaryLabel->setText("No goal set.\nEnter a goal below.");
        m_dailyBudgetLabel->setText("");
        m_bar->clearDailyBudget();
        return;
    }

    QDateTime start = QDateTime::fromString(g.startDateTime, Qt::ISODate);
    QDateTime end   = QDateTime::fromString(g.endDateTime,   Qt::ISODate);
    double    spent = spentSinceGoalStart();
    double    left  = g.totalGoal - spent;
    if (left < 0) left = 0;

    m_goalSummaryLabel->setText(
        QString("Goal: $%1 / %2 %3\nSpent so far: $%4")
            .arg(g.totalGoal,  0, 'f', 2)
            .arg(g.rangeCount)
            .arg(g.rangeUnit)
            .arg(spent, 0, 'f', 2));

    double daily = computeDailyBudget();
    if (daily >= 0) {
        m_dailyBudgetLabel->setText(
            QString("$%1 / day  (remaining: $%2)")
                .arg(daily, 0, 'f', 2)
                .arg(left,  0, 'f', 2));
        m_bar->setDailyBudget(daily);
    } else {
        m_dailyBudgetLabel->setText("Over budget!");
        m_bar->clearDailyBudget();
    }
}

// ── Compute adaptive daily budget ────────────────────────────────────────────
double GoalsTab::computeDailyBudget() const
{
    const GoalData &g = DataStore::instance().goal;
    if (!g.active || g.totalGoal <= 0) return 0;

    QDateTime now  = QDateTime::currentDateTime();
    QDateTime end  = QDateTime::fromString(g.endDateTime, Qt::ISODate);
    double    left = g.totalGoal - spentSinceGoalStart();

    if (left < 0) return -1; // over budget

    // Remaining whole days (including today's remainder)
    double secsLeft = now.secsTo(end);
    double daysLeft = secsLeft / 86400.0;
    if (daysLeft < 1.0) daysLeft = 1.0; // at least 1

    return left / daysLeft;
}

// ── Total spent since goal start ─────────────────────────────────────────────
double GoalsTab::spentSinceGoalStart() const
{
    const GoalData &g = DataStore::instance().goal;
    if (!g.active) return 0;
    QDateTime start = QDateTime::fromString(g.startDateTime, Qt::ISODate);
    QDateTime end   = QDateTime::fromString(g.endDateTime,   Qt::ISODate);
    return DataStore::instance().spentInRange(start, end);
}

// ── Lock / unlock entry box ───────────────────────────────────────────────────
void GoalsTab::lockEntryBox(bool locked)
{
    m_goalAmountEdit->setEnabled(!locked);
    m_rangeCountEdit->setEnabled(!locked);
    m_rangeUnitCombo->setEnabled(!locked);
    m_setGoalBtn->setEnabled(!locked);
    if (locked)
        m_setGoalBtn->setText("Goal Active");
    else
        m_setGoalBtn->setText("Set Goal");
}

// ── Success ───────────────────────────────────────────────────────────────────
void GoalsTab::showCongrats()
{
    GoalData &g = DataStore::instance().goal;
    g.active = false;
    DataStore::instance().save();

    lockEntryBox(false);
    m_goalSummaryLabel->setText("Goal complete! 🎉");
    m_dailyBudgetLabel->setText("");
    m_bar->clearDailyBudget();

    m_congratsFrame->show();
    m_congratsTimer->start(3000);

    emit goalReset();   // tell Tab 1 to clear the pie chart for the new cycle
}

// ── Failure ───────────────────────────────────────────────────────────────────
void GoalsTab::showGoalFailed()
{
    GoalData &g = DataStore::instance().goal;
    g.active = false;
    DataStore::instance().save();

    lockEntryBox(false);
    m_goalSummaryLabel->setText("Budget exceeded.\nPlease set a new goal.");
    m_dailyBudgetLabel->setText("");
    m_bar->clearDailyBudget();

    QMessageBox::warning(this, "Over budget!",
        "You have exceeded your spending goal.\n"
        "Please enter a new goal and time range.");

    emit goalReset();
}