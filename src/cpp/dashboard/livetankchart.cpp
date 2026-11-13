
#include "livetankchart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QSizePolicy>

LiveTankChart::LiveTankChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(230);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void LiveTankChart::addDataPoint(
    const QString &scenario,
    double level
)
{
    if (scenario == "normal_filling")
    {
        normalFillingLevels.append(level);

        if (normalFillingLevels.size() > MAX_POINTS)
            normalFillingLevels.removeFirst();
    }
    else if (scenario == "pump_off_consumption")
    {
        pumpOffConsumptionLevels.append(level);

        if (pumpOffConsumptionLevels.size() > MAX_POINTS)
            pumpOffConsumptionLevels.removeFirst();
    }
    else
    {
        return;
    }

    update();
}

void LiveTankChart::clearData()
{
    normalFillingLevels.clear();
    pumpOffConsumptionLevels.clear();

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
    const int top = 35;
    const int bottom = height() - 45;

    if (right <= left || bottom <= top)
        return;

    painter.setFont(QFont("Segoe UI", 9));

    // Y-axis and grid.
    for (int value = 0; value <= 100; value += 25)
    {
        int y = bottom - (value * (bottom - top) / 100);

        painter.setPen(QPen(QColor("#e2e8f0"), 1));
        painter.drawLine(left, y, right, y);

        painter.setPen(QColor("#64748b"));
        painter.drawText(
            5,
            y + 4,
            QString("%1%").arg(value)
        );
    }

    // Chart legend.
    painter.setPen(QPen(QColor("#2563eb"), 3));
    painter.drawLine(65, 15, 85, 15);

    painter.setPen(QColor("#334155"));
    painter.drawText(90, 19, "Normal Tank Filling");

    painter.setPen(QPen(QColor("#ea580c"), 3));
    painter.drawLine(255, 15, 275, 15);

    painter.setPen(QColor("#334155"));
    painter.drawText(280, 19, "Pump OFF - Consumption");

    if (normalFillingLevels.isEmpty() &&
        pumpOffConsumptionLevels.isEmpty())
    {
        painter.setPen(QColor("#64748b"));
        painter.drawText(
            QRect(left, top, right - left, bottom - top),
            Qt::AlignCenter,
            "Waiting for live tank data..."
        );
        return;
    }

    auto drawHistory =
        [&](const QVector<double> &levels, const QColor &color)
    {
        if (levels.size() < 2)
            return;

        QPolygonF points;

        for (int i = 0; i < levels.size(); ++i)
        {
            double x = left +
                static_cast<double>(i) *
                (right - left) / (MAX_POINTS - 1);

            double clamped = qBound(0.0, levels[i], 100.0);

            double y = bottom -
                (clamped * (bottom - top) / 100.0);

            points << QPointF(x, y);
        }

        painter.setPen(QPen(color, 2.5));
        painter.drawPolyline(points);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        for (const QPointF &point : points)
            painter.drawEllipse(point, 3, 3);
    };

    // Draw both scenario histories.
    drawHistory(normalFillingLevels, QColor("#2563eb"));
    drawHistory(pumpOffConsumptionLevels, QColor("#ea580c"));

    // X-axis label.
    painter.setPen(QColor("#64748b"));
    painter.drawText(
        QRect(left, bottom + 8, right - left, 25),
        Qt::AlignCenter,
        "Simulation Samples"
    );
}