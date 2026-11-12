
#include "livetankchart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>

LiveTankChart::LiveTankChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(230);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void LiveTankChart::addDataPoint(double level)
{
    levels.append(level);

    if (levels.size() > MAX_POINTS)
        levels.removeFirst();

    update();
}

void LiveTankChart::clearData()
{
    levels.clear();
    update();
}

void LiveTankChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor("#ffffff"));

    const int left = 55;
    const int right = width() - 20;
    const int top = 20;
    const int bottom = height() - 35;

    if (right <= left || bottom <= top)
        return;

    painter.setPen(QColor("#64748b"));
    painter.setFont(QFont("Segoe UI", 9));

    for (int value = 0; value <= 100; value += 25)
    {
        int y = bottom - (value * (bottom - top) / 100);

        painter.setPen(QPen(QColor("#e2e8f0"), 1));
        painter.drawLine(left, y, right, y);

        painter.setPen(QColor("#64748b"));
        painter.drawText(5, y + 4, QString("%1%").arg(value));
    }

    if (levels.size() < 2)
    {
        painter.drawText(
            QRect(left, top, right - left, bottom - top),
            Qt::AlignCenter,
            "Waiting for live tank data..."
        );
        return;
    }

    QPolygonF points;

    for (int i = 0; i < levels.size(); ++i)
    {
        double x = left +
            (double)i * (right - left) / (MAX_POINTS - 1);

        double clamped = qBound(0.0, levels[i], 100.0);

        double y = bottom -
            (clamped * (bottom - top) / 100.0);

        points << QPointF(x, y);
    }

    painter.setPen(QPen(QColor("#2563eb"), 2.5));
    painter.drawPolyline(points);

    painter.setBrush(QColor("#2563eb"));
    painter.setPen(Qt::NoPen);

    for (const QPointF &point : points)
        painter.drawEllipse(point, 3, 3);
}