#include "PieChartWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QtMath>
#include <QFontMetrics>

static constexpr double TWO_PI = 2.0 * M_PI;
static constexpr double POP_PX = 14.0;

PieChartWidget::PieChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(300, 300);
}

void PieChartWidget::refresh()
{
    m_hoveredSlice = -1;
    update();
}

QRectF PieChartWidget::chartRect() const
{
    double side = qMin(width(), height()) * 0.78;
    return QRectF((width()  - side) / 2.0,
                  (height() - side) / 2.0,
                  side, side);
}

int PieChartWidget::sliceAt(const QPointF &pos) const
{
    const auto &entries = DataStore::instance().entries;
    if (entries.isEmpty()) return -1;

    QRectF  r  = chartRect();
    QPointF c  = r.center();
    double  rx = r.width()  / 2.0;
    double  ry = r.height() / 2.0;

    double dx = pos.x() - c.x();
    double dy = pos.y() - c.y();
    if ((dx*dx)/(rx*rx) + (dy*dy)/(ry*ry) > 1.0) return -1;

    double total = 0;
    for (const auto &e : entries) total += e.amount;
    if (total <= 0) return -1;

    double angle = qAtan2(-dy, dx);
    if (angle < 0) angle += TWO_PI;
    double cwAngle = M_PI / 2.0 - angle;
    if (cwAngle < 0) cwAngle += TWO_PI;

    double sweep = 0;
    for (int i = 0; i < entries.size(); ++i) {
        double span = (entries[i].amount / total) * TWO_PI;
        if (cwAngle >= sweep && cwAngle < sweep + span)
            return i;
        sweep += span;
    }
    return entries.size() - 1;
}

void PieChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto &entries = DataStore::instance().entries;
    p.fillRect(rect(), palette().window());

    if (entries.isEmpty()) {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "No spending yet.\nAdd an entry below.");
        return;
    }

    double total = 0;
    for (const auto &e : entries) total += e.amount;

    QRectF base = chartRect();
    int qtStart = 90 * 16;

    for (int i = 0; i < entries.size(); ++i) {
        double fraction = entries[i].amount / total;
        int    spanQ    = qRound(-fraction * 360.0 * 16);

        QRectF r = base;
        if (i == m_hoveredSlice) {
            double midAngle = (qtStart / 16.0 + spanQ / 16.0 / 2.0) * M_PI / 180.0;
            r.translate(POP_PX * qCos(midAngle), -POP_PX * qSin(midAngle));
        }

        p.setBrush(entries[i].color);
        p.setPen(Qt::white);
        p.drawPie(r, qtStart, spanQ);

        // ── Hover tooltip ─────────────────────────────────────────────────────
        if (i == m_hoveredSlice) {
            double pct      = fraction * 100.0;
            double midQtDeg = (qtStart + spanQ / 2.0) / 16.0;
            double midRad   = midQtDeg * M_PI / 180.0;
            double rx = r.width()  / 2.0;
            double ry = r.height() / 2.0;
            double tipX = r.center().x() + (rx + POP_PX + 6) * qCos(midRad);
            double tipY = r.center().y() - (ry + POP_PX + 6) * qSin(midRad);

            // Three lines: title | $amount | percentage
            QString label = QString("%1\n$%2\n%3%")
                                .arg(entries[i].title)
                                .arg(entries[i].amount, 0, 'f', 2)
                                .arg(pct, 0, 'f', 1);

            QFont font = p.font();
            font.setPointSize(9);
            font.setBold(true);
            p.setFont(font);
            QFontMetrics fm(font);
            QStringList lines = label.split('\n');
            int tw = 0;
            for (const auto &l : lines) tw = qMax(tw, fm.horizontalAdvance(l));
            int th = fm.height() * lines.size();

            QRectF box(tipX - tw / 2.0 - 8, tipY - th / 2.0 - 6,
                       tw + 16, th + 12);
            if (box.right()  > width()  - 4) box.moveRight(width()  - 4);
            if (box.left()   < 4)             box.moveLeft(4);
            if (box.bottom() > height() - 4)  box.moveBottom(height() - 4);
            if (box.top()    < 4)             box.moveTop(4);

            p.setPen(QPen(entries[i].color.darker(140), 1.5));
            p.setBrush(QColor(255, 255, 255, 230));
            p.drawRoundedRect(box, 6, 6);

            p.setPen(Qt::black);
            for (int li = 0; li < lines.size(); ++li) {
                QRectF lr(box.left(),
                          box.top() + 6 + li * fm.height(),
                          box.width(), fm.height());
                p.drawText(lr, Qt::AlignHCenter | Qt::AlignVCenter, lines[li]);
            }
        }

        qtStart += spanQ;
    }
}

void PieChartWidget::mouseMoveEvent(QMouseEvent *ev)
{
    int idx = sliceAt(ev->position());
    if (idx != m_hoveredSlice) { m_hoveredSlice = idx; update(); }
}

void PieChartWidget::leaveEvent(QEvent *)
{
    if (m_hoveredSlice != -1) { m_hoveredSlice = -1; update(); }
}
