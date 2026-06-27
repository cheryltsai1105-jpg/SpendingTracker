#pragma once
#include <QWidget>
#include "DataStore.h"

// ─── Interactive pie chart drawn with QPainter ───────────────────────────────
// Hover over a slice → a tooltip "tab" pops out from the slice edge.
class PieChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget *parent = nullptr);

    void refresh();                 // call after entries change

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    int     m_hoveredSlice = -1;    // index into DataStore::entries, or ‑1

    // geometry helpers
    QRectF  chartRect() const;
    int     sliceAt(const QPointF &pos) const;  // hit‑test → index or ‑1
};
