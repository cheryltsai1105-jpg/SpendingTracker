#pragma once
#include <QWidget>

class PieChartWidget;
class QLabel;
class QLineEdit;
class QPushButton;

class TrackerTab : public QWidget
{
    Q_OBJECT
public:
    explicit TrackerTab(QWidget *parent = nullptr);

    // Called externally when a goal resets so the pie can be cleared
    void resetPieChart();

signals:
    // Emitted after a new entry is successfully saved
    void entryAdded();

private slots:
    void onAddEntry();

private:
    PieChartWidget *m_pie;
    QLabel         *m_totalLabel;
    QLineEdit      *m_titleEdit;
    QLineEdit      *m_amountEdit;
    QPushButton    *m_addBtn;

    void updateTotal();
    static QColor randomColor();
};
