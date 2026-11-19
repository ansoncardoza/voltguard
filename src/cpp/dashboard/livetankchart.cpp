#include "livetankchart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <functional>
#include <QFont>
#include <QPolygonF>
#include <QSizePolicy>
#include <QRectF>
#include <QtGlobal>
#include <limits>

#include <algorithm>
#include <cmath>

LiveTankChart::LiveTankChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(460);
    setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
}

void LiveTankChart::addDataPoint(
    const QString &scenario,
    const QString &timestamp,
    int sequence,
    double level,
    double volume,
    double inflow,
    double outflow,
    bool pumpOn
)
{
    TelemetryPoint point{
        timestamp,
        sequence,
        level,
        volume,
        inflow,
        outflow,
        pumpOn
    };

    QVector<TelemetryPoint> *history = nullptr;

    if (scenario == "normal_filling")
    {
        history = &normalFillingHistory;
    }
    else if (scenario == "pump_off_consumption")
    {
        history = &pumpOffConsumptionHistory;
    }
    else
    {
        return;
    }

    history->append(point);

    const int maxPoints =
    (scenario == "normal_filling")
        ? MAX_NORMAL_POINTS
        : MAX_PUMP_OFF_POINTS;

    if (history->size() > maxPoints)
    history->removeFirst();

    update();
}

void LiveTankChart::addEvent(
    const QString &timestamp,
    const QString &eventType,
    const QString &label,
    const QString &status,
    int sequence
)
{
    TimelineEvent event{
        timestamp,
        eventType,
        label,
        status,
        sequence
    };

    eventHistory.append(event);

    if (eventHistory.size() > MAX_EVENTS)
        eventHistory.removeFirst();

    update();
}

void LiveTankChart::clearData()
{
    normalFillingHistory.clear();
    pumpOffConsumptionHistory.clear();
    eventHistory.clear();

    update();
}

void LiveTankChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor background("#101820");
    const QColor panel("#17232e");
    const QColor grid("#30414f");
    const QColor text("#dce6ee");
    const QColor muted("#91a3b2");

    const QColor normalColor("#38bdf8");
    const QColor consumptionColor("#fb923c");

    painter.fillRect(rect(), background);

    const int left = 82;
    const int right = width() - 24;
    const int top = 58;
    const int bottom = height() - 30;

    if (right <= left || bottom <= top)
        return;

    painter.setFont(QFont("Segoe UI", 10));

    // -------------------------------------------------
    // HEADER
    // -------------------------------------------------

    painter.setPen(text);
    painter.setFont(QFont("Segoe UI", 11, QFont::DemiBold));

    painter.drawText(
        18,
        25,
        "LIVE WATER SYSTEM TELEMETRY"
    );

    painter.setFont(QFont("Segoe UI", 8));

    painter.setPen(QPen(normalColor, 3));
    painter.drawLine(300, 22, 320, 22);

    painter.setPen(text);
    painter.drawText(326, 26, "Normal Filling");

    painter.setPen(QPen(consumptionColor, 3));
    painter.drawLine(440, 22, 460, 22);

    painter.setPen(text);
    painter.drawText(466, 26, "Pump OFF / Consumption");

    if (normalFillingHistory.isEmpty() &&
        pumpOffConsumptionHistory.isEmpty() &&
        eventHistory.isEmpty())
    {
        painter.setPen(muted);
        painter.drawText(
            rect(),
            Qt::AlignCenter,
            "Waiting for live telemetry..."
        );

        return;
    }

    // -------------------------------------------------
    // DATA RANGE
    // -------------------------------------------------

    double maxVolume = 100.0;
    double maxFlow = 10.0;

    auto inspectHistory =
        [&](const QVector<TelemetryPoint> &history)
    {
        for (const TelemetryPoint &point : history)
        {
            maxVolume = qMax(maxVolume, point.volume);
            maxFlow = qMax(maxFlow, point.inflow);
            maxFlow = qMax(maxFlow, point.outflow);
        }
    };

    inspectHistory(normalFillingHistory);
    inspectHistory(pumpOffConsumptionHistory);

    maxVolume =
        qMax(100.0, std::ceil(maxVolume / 100.0) * 100.0);

    maxFlow =
        qMax(10.0, std::ceil(maxFlow / 10.0) * 10.0);

    // -------------------------------------------------
    // TRACK LAYOUT
    // -------------------------------------------------

    const int trackGap = 7;
    const int eventTrackHeight = 75;

    const int telemetryAreaHeight =
    bottom - top - eventTrackHeight - (trackGap * 4);

    const int trackHeight = telemetryAreaHeight / 4;

    if (trackHeight < 35)
    return;

    const QRectF tracks[4] = {
    QRectF(left, top, right - left, trackHeight),

    QRectF(
        left,
        top + trackHeight + trackGap,
        right - left,
        trackHeight
    ),

    QRectF(
        left,
        top + (trackHeight + trackGap) * 2,
        right - left,
        trackHeight
    ),

    QRectF(
        left,
        top + (trackHeight + trackGap) * 3,
        right - left,
        trackHeight
    )
    };

    const QRectF eventTrack(
    left,
    tracks[3].bottom() + trackGap,
    right - left,
    eventTrackHeight
    );

    // -------------------------------------------------
    // DRAW TRACK BACKGROUNDS AND GRID
    // -------------------------------------------------

    for (const QRectF &track : tracks)
    {
        painter.fillRect(track, panel);

        painter.setPen(QPen(grid, 1));

        for (int i = 0; i <= 2; ++i)
        {
            double y =
                track.top() + (track.height() * i / 2.0);

            painter.drawLine(
                QPointF(track.left(), y),
                QPointF(track.right(), y)
            );
        }
    }

    // -------------------------------------------------
    // EVENT TIMELINE BACKGROUND
    // -------------------------------------------------

    painter.fillRect(eventTrack, panel);

    painter.setPen(QPen(grid, 1));

    for (int i = 0; i <= 1; ++i)
    {
    const double y =
        eventTrack.top() +
        eventTrack.height() * i;

    painter.drawLine(
        QPointF(eventTrack.left(), y),
        QPointF(eventTrack.right(), y)
    );
    }

    // -------------------------------------------------
    // TRACK LABELS
    // -------------------------------------------------

    painter.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
    painter.setPen(text);

    painter.drawText(
        QRectF(12, tracks[0].top(), 65, tracks[0].height()),
        Qt::AlignVCenter | Qt::AlignLeft,
        "LEVEL\n(%)"
    );

    painter.drawText(
        QRectF(12, tracks[1].top(), 65, tracks[1].height()),
        Qt::AlignVCenter | Qt::AlignLeft,
        "VOLUME\n(L)"
    );

    painter.drawText(
        QRectF(12, tracks[2].top(), 65, tracks[2].height()),
        Qt::AlignVCenter | Qt::AlignLeft,
        "FLOW\n(L/min)"
    );

    painter.drawText(
        QRectF(12, tracks[3].top(), 65, tracks[3].height()),
        Qt::AlignVCenter | Qt::AlignLeft,
        "PUMP\nSTATE"
    );

    // -------------------------------------------------
    // EVENT TIMELINE LABEL
    // -------------------------------------------------

    painter.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
    painter.setPen(text);

    painter.drawText(
    QRectF(
        12,
        eventTrack.top(),
        65,
        eventTrack.height()
    ),
    Qt::AlignVCenter | Qt::AlignLeft,
    "EVENT\nTIMELINE"
    );

    // -------------------------------------------------
    // Y-AXIS VALUES
    // -------------------------------------------------

    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(muted);

    painter.drawText(
        QRectF(48, tracks[0].top() - 5, 30, 12),
        Qt::AlignRight,
        "100"
    );

    painter.drawText(
        QRectF(48, tracks[0].bottom() - 7, 30, 12),
        Qt::AlignRight,
        "0"
    );

    painter.drawText(
        QRectF(48, tracks[1].top() - 5, 30, 12),
        Qt::AlignRight,
        QString::number(maxVolume, 'f', 0)
    );

    painter.drawText(
        QRectF(48, tracks[1].bottom() - 7, 30, 12),
        Qt::AlignRight,
        "0"
    );

    painter.drawText(
        QRectF(48, tracks[2].top() - 5, 30, 12),
        Qt::AlignRight,
        QString::number(maxFlow, 'f', 0)
    );

    painter.drawText(
        QRectF(48, tracks[2].bottom() - 7, 30, 12),
        Qt::AlignRight,
        "0"
    );

    painter.drawText(
        QRectF(48, tracks[3].top() - 5, 30, 12),
        Qt::AlignRight,
        "ON"
    );

    painter.drawText(
        QRectF(48, tracks[3].bottom() - 7, 30, 12),
        Qt::AlignRight,
        "OFF"
    );

    // -------------------------------------------------
    // DRAW TELEMETRY HISTORY
    // -------------------------------------------------

    // -------------------------------------------------
    // SHARED TIMELINE POSITIONING
    // -------------------------------------------------

int minSequence = std::numeric_limits<int>::max();
int maxSequence = std::numeric_limits<int>::min();

auto includeTelemetrySequences =
    [&](const QVector<TelemetryPoint> &history)
{
    for (const TelemetryPoint &point : history)
    {
        minSequence = qMin(minSequence, point.sequence);
        maxSequence = qMax(maxSequence, point.sequence);
    }
};

includeTelemetrySequences(normalFillingHistory);
includeTelemetrySequences(pumpOffConsumptionHistory);

for (const TimelineEvent &event : eventHistory)
{
    minSequence = qMin(minSequence, event.sequence);
    maxSequence = qMax(maxSequence, event.sequence);
}

auto timelineX =
    [&](const QRectF &track, int sequence) -> double
{
    if (minSequence == std::numeric_limits<int>::max())
        return track.left();

    if (maxSequence == minSequence)
        return track.left() + track.width();

    const double progress =
        static_cast<double>(sequence - minSequence) /
        static_cast<double>(maxSequence - minSequence);

    return track.left() + progress * track.width();
};
    
    auto drawLine =
        [&](const QVector<TelemetryPoint> &history,
            const QRectF &track,
            const QColor &color,
            const std::function<double(const TelemetryPoint &)> &valueGetter,
            double maximum,
            Qt::PenStyle style = Qt::SolidLine)
    {
        if (history.size() < 2)
            return;

        QPolygonF points;

        for (int i = 0; i < history.size(); ++i)
        {
            const double x =
            timelineX(track, history[i].sequence);

            const double value =
                qBound(0.0, valueGetter(history[i]), maximum);

            const double y =
                track.bottom() -
                (value / maximum) * track.height();

            points << QPointF(x, y);
        }

        painter.setPen(QPen(color, 2.2, style));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolyline(points);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        for (const QPointF &point : points)
            painter.drawEllipse(point, 2.5, 2.5);
    };

    auto levelValue =
        [](const TelemetryPoint &point)
    {
        return point.level;
    };

    auto volumeValue =
        [](const TelemetryPoint &point)
    {
        return point.volume;
    };

    auto inflowValue =
        [](const TelemetryPoint &point)
    {
        return point.inflow;
    };

    auto outflowValue =
        [](const TelemetryPoint &point)
    {
        return point.outflow;
    };

    // Tank level.

    drawLine(
        normalFillingHistory,
        tracks[0],
        normalColor,
        levelValue,
        100.0
    );

    drawLine(
        pumpOffConsumptionHistory,
        tracks[0],
        consumptionColor,
        levelValue,
        100.0
    );

    // Tank volume.

    drawLine(
        normalFillingHistory,
        tracks[1],
        normalColor,
        volumeValue,
        maxVolume
    );

    drawLine(
        pumpOffConsumptionHistory,
        tracks[1],
        consumptionColor,
        volumeValue,
        maxVolume
    );

    // Inflow: solid lines.

    drawLine(
        normalFillingHistory,
        tracks[2],
        normalColor,
        inflowValue,
        maxFlow
    );

    drawLine(
        pumpOffConsumptionHistory,
        tracks[2],
        consumptionColor,
        inflowValue,
        maxFlow
    );

    // Outflow: dashed lines.

    drawLine(
        normalFillingHistory,
        tracks[2],
        normalColor,
        outflowValue,
        maxFlow,
        Qt::DashLine
    );

    drawLine(
        pumpOffConsumptionHistory,
        tracks[2],
        consumptionColor,
        outflowValue,
        maxFlow,
        Qt::DashLine
    );

    // -------------------------------------------------
    // PUMP ON / OFF STATE
    // -------------------------------------------------

    auto drawPumpState =
        [&](const QVector<TelemetryPoint> &history,
            const QColor &color)
    {
        if (history.isEmpty())
            return;

        const double onY = tracks[3].top() + 8;
        const double offY = tracks[3].bottom() - 8;

        QPolygonF points;

        for (int i = 0; i < history.size(); ++i)
        {
            const double x =
            timelineX(tracks[3], history[i].sequence);

            const double y =
                history[i].pumpOn ? onY : offY;

            points << QPointF(x, y);
        }

        painter.setPen(QPen(color, 2.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolyline(points);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        for (const QPointF &point : points)
            painter.drawEllipse(point, 3, 3);
    };

    drawPumpState(normalFillingHistory, normalColor);
    drawPumpState(pumpOffConsumptionHistory, consumptionColor);

    // -------------------------------------------------
    // X-AXIS
    // -------------------------------------------------

    // -------------------------------------------------
    // DRAW MODBUS, PHYSICS AND DECISION EVENTS
    // -------------------------------------------------

const int markerRows = 3;

auto eventColor = [](const TimelineEvent &event)
{
    if (event.eventType == "DECISION")
    {
        if (event.status == "DROP")
            return QColor("#ef4444");

        if (event.status == "HOLD")
            return QColor("#f59e0b");

        if (event.status == "ALLOW")
            return QColor("#22c55e");
    }

    if (event.eventType == "MODBUS")
    {
        if (event.status == "INVALID_FRAME")
            return QColor("#ef4444");

        if (event.status == "VALIDATION_FAILED")
            return QColor("#f59e0b");

        if (event.status == "VALID")
            return QColor("#22c55e");
    }

    if (event.eventType == "PHYSICS")
    {
        if (event.status == "INVALID")
            return QColor("#ef4444");

        if (event.status == "WARNING")
            return QColor("#f59e0b");

        if (event.status == "VALID")
            return QColor("#22c55e");
    }

    return QColor("#91a3b2");
};

auto eventRow = [](const QString &eventType)
{
    if (eventType == "MODBUS")
        return 0;

    if (eventType == "PHYSICS")
        return 1;

    if (eventType == "DECISION")
        return 2;

    return -1;
};

const int markerSpacing =
    static_cast<int>(eventTrack.height()) / markerRows;

for (const TimelineEvent &event : eventHistory)
{
    const int row = eventRow(event.eventType);

    if (row < 0)
        continue;

    const double x =
        timelineX(eventTrack, event.sequence);

    const double y =
        eventTrack.top() +
        markerSpacing * row +
        markerSpacing / 2.0;

    const QColor color = eventColor(event);

    QString markerLabel;

    if (event.eventType == "MODBUS")
        markerLabel = "M";
    else if (event.eventType == "PHYSICS")
        markerLabel = "P";
    else if (event.eventType == "DECISION")
        markerLabel = "D";

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    painter.drawEllipse(
        QPointF(x, y),
        7,
        7
    );

    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 7, QFont::Bold));

    painter.drawText(
        QRectF(x - 7, y - 7, 14, 14),
        Qt::AlignCenter,
        markerLabel
    );
}

    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(muted);

    painter.drawText(
        QRectF(
            left,
            tracks[3].bottom() + 5,
            right - left,
            20
        ),
        Qt::AlignCenter,
        "LIVE SIMULATION SAMPLES  →"
    );
}