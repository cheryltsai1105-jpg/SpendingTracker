#pragma once
#include <QWidget>
#include <QDate>

class BarChartWidget;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTimer;
class QFrame;

class GoalsTab : public QWidget
{
    Q_OBJECT
public:
    explicit GoalsTab(QWidget *parent = nullptr);

    // Called by MainWindow after a new entry is added in Tab 1
    void onEntryAdded();

signals:
    // Emitted when the goal cycle resets so Tab 1 can clear its pie chart
    void goalReset();

private slots:
    void onSetGoal();
    void checkGoalStatus();

private:
    // ── Bar chart ─────────────────────────────────────────────────────────────
    BarChartWidget *m_bar;
    QLabel         *m_weekRangeLabel;   // "Week: Jun 09 – Jun 15"

    // ── Info panel (right side) ───────────────────────────────────────────────
    QLabel         *m_goalSummaryLabel; // "Goal: $500 / 1 week"
    QLabel         *m_dailyBudgetLabel; // "$71.43 / day remaining"

    // ── Entry panel ───────────────────────────────────────────────────────────
    QFrame         *m_entryFrame;
    QLineEdit      *m_goalAmountEdit;
    QLineEdit      *m_rangeCountEdit;
    QComboBox      *m_rangeUnitCombo;
    QPushButton    *m_setGoalBtn;

    // ── Congrats overlay ──────────────────────────────────────────────────────
    QFrame         *m_congratsFrame;
    QTimer         *m_congratsTimer;

    // ── Helpers ───────────────────────────────────────────────────────────────
    void lockEntryBox(bool locked);
    void updateInfoPanel();
    void advanceBarChartWeekIfNeeded();
    void showCongrats();
    void showGoalFailed();

    // Recompute daily budget = remaining goal / remaining days in goal window
    double computeDailyBudget() const;

    // Total spent since goal start
    double spentSinceGoalStart() const;
};
