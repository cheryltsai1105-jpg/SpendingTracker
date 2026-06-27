#pragma once
#include <QWidget>
#include <QDate>

// ── Weekly bar chart for the Goals & Analysis tab ────────────────────────────
// Shows Mon–Sun for the currently-displayed week.
// Bars reflect spending pulled from DataStore for each calendar day.
// Hovering a bar shows a tooltip with the day's total.
// An optional red dashed "daily budget" line is drawn across the chart.
class BarChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget *parent = nullptr);

    // Set which Mon–Sun week to display (Monday of the week).
    void setWeekStart(const QDate &monday);
    QDate weekStart() const { return m_weekStart; }

    // Redraw (call after new transactions arrive).
    void refresh();

    // Draw or hide the horizontal daily-budget guide line.
    void setDailyBudget(double budget);   // pass 0 to hide
    void clearDailyBudget();

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    QDate  m_weekStart;       // always a Monday
    double m_dailyBudget = 0; // 0 = hidden
    int    m_hoveredBar  = -1; // 0-6

    // Map bar index → pixel rect
    QRect barRect(int idx, double maxVal, const QRect &plotArea) const;
};
