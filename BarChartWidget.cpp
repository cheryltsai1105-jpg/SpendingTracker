#include "BarChartWidget.h"
#include "DataStore.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QtMath>

static const QColor BAR_COLOR      (70,  130, 220);
static const QColor BAR_HOVER_COLOR(50,  100, 190);
static const QColor BUDGET_LINE_CLR(220,  60,  60);
static const char  *DAY_NAMES[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

BarChartWidget::BarChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(300, 220);
}

void BarChartWidget::setWeekStart(const QDate &monday)
{
    m_weekStart   = monday;
    m_hoveredBar  = -1;
    update();
}

void BarChartWidget::refresh()          { update(); }
void BarChartWidget::setDailyBudget(double b) { m_dailyBudget = b; update(); }
void BarChartWidget::clearDailyBudget()       { m_dailyBudget = 0; update(); }

// ── Plot area (inside axes) ───────────────────────────────────────────────────
static QRect plotArea(const QRect &widget)
{
    return QRect(widget.left() + 52, widget.top() + 14,
                 widget.width() - 66, widget.height() - 54);
}

// ── Bar rect for bar index i (0=Mon … 6=Sun) ─────────────────────────────────
QRect BarChartWidget::barRect(int idx, double maxVal, const QRect &pa) const
{
    if (m_weekStart.isNull()) return {};
    double spent = DataStore::instance().spentOnDate(m_weekStart.addDays(idx));
    if (maxVal <= 0) maxVal = 1;

    int barW   = qMax(8, (pa.width() - 14) / 7 - 6);
    int xStep  = (pa.width() - 14) / 7;
    int x      = pa.left() + 7 + idx * xStep + (xStep - barW) / 2;
    int barH   = qRound((spent / maxVal) * pa.height());
    int y      = pa.bottom() - barH;
    return QRect(x, y, barW, barH);
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void BarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().window());

    QRect pa = plotArea(rect());

    // Collect daily totals
    double vals[7] = {}, maxVal = 0;
    if (!m_weekStart.isNull()) {
        for (int i = 0; i < 7; ++i) {
            vals[i] = DataStore::instance().spentOnDate(m_weekStart.addDays(i));
            maxVal  = qMax(maxVal, vals[i]);
        }
    }
    // Ensure budget line fits
    if (m_dailyBudget > 0) maxVal = qMax(maxVal, m_dailyBudget);
    if (maxVal <= 0) maxVal = 100;

    // ── Y-axis labels (4 ticks) ───────────────────────────────────────────────
    p.setPen(QColor(140,140,140));
    QFont small = p.font(); small.setPointSize(8); p.setFont(small);
    for (int t = 0; t <= 4; ++t) {
        double val = maxVal * t / 4.0;
        int    y   = pa.bottom() - qRound((val / maxVal) * pa.height());
        p.drawLine(pa.left() - 4, y, pa.right(), y);
        p.drawText(QRect(0, y - 9, pa.left() - 6, 18),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString("$%1").arg(val, 0, 'f', 0));
    }

    // ── Axes ─────────────────────────────────────────────────────────────────
    p.setPen(QPen(Qt::darkGray, 1.5));
    p.drawLine(pa.left(), pa.top(),    pa.left(),  pa.bottom());
    p.drawLine(pa.left(), pa.bottom(), pa.right(), pa.bottom());

    // ── Budget line ───────────────────────────────────────────────────────────
    if (m_dailyBudget > 0) {
        int by = pa.bottom() - qRound((m_dailyBudget / maxVal) * pa.height());
        QPen dashed(BUDGET_LINE_CLR, 1.5, Qt::DashLine);
        p.setPen(dashed);
        p.drawLine(pa.left(), by, pa.right(), by);
        // label
        QFont lf = p.font(); lf.setPointSize(8); lf.setBold(true); p.setFont(lf);
        p.setPen(BUDGET_LINE_CLR);
        p.drawText(pa.right() - 60, by - 3, QString("$%1/day").arg(m_dailyBudget, 0,'f',2));
    }

    // ── Bars ─────────────────────────────────────────────────────────────────
    QFont labFont = p.font(); labFont.setPointSize(8); labFont.setBold(false);
    p.setFont(labFont);

    for (int i = 0; i < 7; ++i) {
        QRect br = barRect(i, maxVal, pa);
        if (br.height() > 0) {
            p.setBrush(i == m_hoveredBar ? BAR_HOVER_COLOR : BAR_COLOR);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(br, 3, 3);
        }

        // X-axis label: "Mon\n06/16"
        int xStep = (pa.width() - 14) / 7;
        int cx    = pa.left() + 7 + i * xStep + xStep / 2;
        QString dayName = DAY_NAMES[i];
        QString dateStr = m_weekStart.isNull() ? "--/--"
                        : m_weekStart.addDays(i).toString("MM/dd");

        p.setPen(Qt::darkGray);
        p.drawText(QRect(cx - 24, pa.bottom() + 4,  48, 14),
                   Qt::AlignHCenter, dayName);
        p.drawText(QRect(cx - 24, pa.bottom() + 18, 48, 14),
                   Qt::AlignHCenter, dateStr);
    }

    // ── Hover tooltip ─────────────────────────────────────────────────────────
    if (m_hoveredBar >= 0 && !m_weekStart.isNull()) {
        double spent = vals[m_hoveredBar];
        QRect  br    = barRect(m_hoveredBar, maxVal, pa);
        int    tipY  = (br.height() > 0) ? br.top() - 6 : pa.bottom() - 6;
        int    tipX  = br.left() + br.width() / 2;

        QString txt = QString("$%1").arg(spent, 0, 'f', 2);
        QFont tf = p.font(); tf.setPointSize(9); tf.setBold(true); p.setFont(tf);
        QFontMetrics fm(tf);
        int tw = fm.horizontalAdvance(txt) + 16;
        int th = fm.height() + 10;
        QRect box(tipX - tw / 2, tipY - th, tw, th);
        if (box.right()  > rect().right()  - 4) box.moveRight(rect().right() - 4);
        if (box.left()   < rect().left()   + 4) box.moveLeft(rect().left() + 4);
        if (box.top()    < rect().top()    + 4) box.moveTop(rect().top() + 4);

        p.setPen(QPen(BAR_COLOR.darker(130), 1.5));
        p.setBrush(QColor(255, 255, 255, 230));
        p.drawRoundedRect(box, 5, 5);
        p.setPen(Qt::black);
        p.drawText(box, Qt::AlignCenter, txt);
    }
}

// ── Mouse tracking ────────────────────────────────────────────────────────────
void BarChartWidget::mouseMoveEvent(QMouseEvent *ev)
{
    QRect pa = plotArea(rect());
    if (m_weekStart.isNull() || !pa.contains(ev->pos())) {
        if (m_hoveredBar != -1) { m_hoveredBar = -1; update(); }
        return;
    }
    int xStep = (pa.width() - 14) / 7;
    int idx   = (ev->pos().x() - pa.left() - 7) / xStep;
    idx = qBound(0, idx, 6);
    if (idx != m_hoveredBar) { m_hoveredBar = idx; update(); }
}

void BarChartWidget::leaveEvent(QEvent *)
{
    if (m_hoveredBar != -1) { m_hoveredBar = -1; update(); }
}
