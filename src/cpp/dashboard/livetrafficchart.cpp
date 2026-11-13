
#include "livetrafficchart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QFont>
#include <QSizePolicy>
#include <algorithm>

LiveTrafficChart::LiveTrafficChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void LiveTrafficChart::addDataPoint(
    const QString &scenario,
    int valid,
    int validationFailures,
    int invalid
)
{
    TrafficSnapshot snapshot{
        valid,
        validationFailures,
        invalid
    };

    if (scenario == "normal_filling")
    {
        normalFillingHistory.append(snapshot);

        if (normalFillingHistory.size() > MAX_POINTS)
            normalFillingHistory.removeFirst();
    }
    else if (scenario == "pump_off_consumption")
    {
        pumpOffConsumptionHistory.append(snapshot);

        if (pumpOffConsumptionHistory.size() > MAX_POINTS)
            pumpOffConsumptionHistory.removeFirst();
    }
    else if (scenario == "invalid_traffic")
{
    invalidTrafficHistory.append(snapshot);

    if (invalidTrafficHistory.size() > MAX_POINTS)
        invalidTrafficHistory.removeFirst();
}
    else
    {
        return;
    }

    update();
}

void LiveTrafficChart::clearData()
{
    normalFillingHistory.clear();
    pumpOffConsumptionHistory.clear();
    invalidTrafficHistory.clear();
    update();
}

void LiveTrafficChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#ffffff"));

    const int left = 55;
    const int right = width() - 20;
    const int top = 40;
    const int bottom = height() - 45;

    if (right <= left || bottom <= top)
        return;

    int maxValue = 1;

    auto inspectHistory = [&](const QVector<TrafficSnapshot> &history)
    {
        for (const auto &sample : history)
        {
            maxValue = std::max(maxValue, sample.valid);
            maxValue = std::max(maxValue, sample.validationFailures);
            maxValue = std::max(maxValue, sample.invalid);
        }
    };

    inspectHistory(normalFillingHistory);
    inspectHistory(pumpOffConsumptionHistory);
    inspectHistory(invalidTrafficHistory);

    // Round the vertical scale up to a readable value.
    int scaleMaximum = ((maxValue + 4) / 5) * 5;
    if (scaleMaximum < 5)
        scaleMaximum = 5;

    painter.setFont(QFont("Segoe UI", 9));

    // Grid and Y-axis labels.
    for (int i = 0; i <= 4; ++i)
    {
        int value = scaleMaximum * i / 4;
        int y = bottom - i * (bottom - top) / 4;

        painter.setPen(QPen(QColor("#e2e8f0"), 1));
        painter.drawLine(left, y, right, y);

        painter.setPen(QColor("#64748b"));
        painter.drawText(5, y + 4, QString::number(value));
    }

    // Legend.
    painter.setPen(QPen(QColor("#16a34a"), 3));
    painter.drawLine(60, 17, 80, 17);

    painter.setPen(QPen(QColor("#047857"), 3, Qt::DashLine));
    painter.drawLine(85, 17, 105, 17);

    painter.setPen(QColor("#334155"));
    painter.drawText(110, 21, "Valid");

    painter.setPen(QPen(QColor("#ea580c"), 3));
    painter.drawLine(145, 17, 165, 17);
    painter.setPen(QColor("#334155"));
    painter.drawText(170, 21, "Validation Failures");

    painter.setPen(QPen(QColor("#dc2626"), 3));
    painter.drawLine(295, 17, 315, 17);
    painter.setPen(QColor("#334155"));
    painter.drawText(320, 21, "Invalid");

    if (normalFillingHistory.isEmpty() &&
        pumpOffConsumptionHistory.isEmpty()&&
        invalidTrafficHistory.isEmpty())
    {
        painter.setPen(QColor("#64748b"));
        painter.drawText(
            QRect(left, top, right - left, bottom - top),
            Qt::AlignCenter,
            "Waiting for live Modbus traffic..."
        );
        return;
    }

    auto drawSeries =
        [&](const QVector<TrafficSnapshot> &history,
            int TrafficSnapshot::*field,
            const QColor &color,
            Qt::PenStyle style)
    {
        if (history.isEmpty())
            return;

        QPolygonF points;

        for (int i = 0; i < history.size(); ++i)
        {
            double x = left;

            if (history.size() > 1)
            {
                x += static_cast<double>(i) *
                    (right - left) / (MAX_POINTS - 1);
            }

            int value = history[i].*field;

            double y = bottom -
                static_cast<double>(value) *
                (bottom - top) / scaleMaximum;

            points << QPointF(x, y);
        }

        painter.setPen(QPen(color, 2.5, style));

        if (points.size() >= 2)
            painter.drawPolyline(points);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        for (const QPointF &point : points)
            painter.drawEllipse(point, 3, 3);
    };

    // Normal tank filling: solid lines.
    drawSeries(
        normalFillingHistory,
        &TrafficSnapshot::valid,
        QColor("#16a34a"),
        Qt::SolidLine
    );

    drawSeries(
        normalFillingHistory,
        &TrafficSnapshot::validationFailures,
        QColor("#ea580c"),
        Qt::SolidLine
    );

    drawSeries(
        normalFillingHistory,
        &TrafficSnapshot::invalid,
        QColor("#dc2626"),
        Qt::SolidLine
    );

    // Pump OFF consumption: dashed lines.
    drawSeries(
        pumpOffConsumptionHistory,
        &TrafficSnapshot::valid,
        QColor("#047857"),
        Qt::DashLine
    );

    drawSeries(
        pumpOffConsumptionHistory,
        &TrafficSnapshot::validationFailures,
        QColor("#ea580c"),
        Qt::DashLine
    );

    drawSeries(
        pumpOffConsumptionHistory,
        &TrafficSnapshot::invalid,
        QColor("#dc2626"),
        Qt::DashLine
    );
    // Invalid Modbus traffic: red dotted lines.
drawSeries(
    invalidTrafficHistory,
    &TrafficSnapshot::valid,
    QColor("#dc2626"),
    Qt::DotLine
);

drawSeries(
    invalidTrafficHistory,
    &TrafficSnapshot::validationFailures,
    QColor("#ea580c"),
    Qt::DotLine
);

drawSeries(
    invalidTrafficHistory,
    &TrafficSnapshot::invalid,
    QColor("#b91c1c"),
    Qt::SolidLine
);

    painter.setPen(QColor("#64748b"));
    painter.drawText(
        QRect(left, bottom + 8, right - left, 25),
        Qt::AlignCenter,
        "Live Sample Sequence"
    );
}
