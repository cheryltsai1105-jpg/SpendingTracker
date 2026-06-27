#pragma once
#include <QMainWindow>
#include <QTabWidget>

class TrackerTab;
class GoalsTab;
class HistoryTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QTabWidget  *m_tabs;
    TrackerTab  *m_trackerTab;
    GoalsTab    *m_goalsTab;
    HistoryTab  *m_historyTab;
};
