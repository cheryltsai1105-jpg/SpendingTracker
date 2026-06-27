#include "MainWindow.h"
#include "TrackerTab.h"
#include "GoalsTab.h"
#include "HistoryTab.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_tabs = new QTabWidget(this);

    m_trackerTab = new TrackerTab(this);
    m_goalsTab   = new GoalsTab(this);
    m_historyTab = new HistoryTab(this);

    m_tabs->addTab(m_trackerTab, "Tracker");
    m_tabs->addTab(m_goalsTab,   "Goals & Analysis");
    m_tabs->addTab(m_historyTab, "History");

    setCentralWidget(m_tabs);

    // ── Wire cross-tab signals ────────────────────────────────────────────────

    // Tab 1 → Tab 3 FIRST: history must record the entry before anything else
    // runs (goal check can clear DataStore::entries via goalReset, so history
    // must grab the entry while it is still present).
    connect(m_trackerTab, &TrackerTab::entryAdded,
            m_historyTab, &HistoryTab::appendLatestEntry);

    // Tab 1 → Tab 2: update bar chart & check goal status (may emit goalReset)
    connect(m_trackerTab, &TrackerTab::entryAdded,
            m_goalsTab,   &GoalsTab::onEntryAdded);

    // Tab 2 → Tab 1: goal reset (success or fail) → clear pie chart for new cycle
    connect(m_goalsTab,   &GoalsTab::goalReset,
            m_trackerTab, &TrackerTab::resetPieChart);
}