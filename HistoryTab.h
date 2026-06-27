#pragma once
#include <QWidget>

class QTableWidget;

// ── Tab 3: permanent, ever-growing transaction log ────────────────────────────
// Columns: Title | Amount | Date
// Rows are appended forever; data is stored in a separate QSettings key
// so it is never cleared by goal resets or pie chart resets.
class HistoryTab : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryTab(QWidget *parent = nullptr);

    // Append the most-recent DataStore entry to the table and persist it.
    void appendLatestEntry();

private:
    QTableWidget *m_table;

    void loadFromSettings();
    void saveAllToSettings() const;
};
