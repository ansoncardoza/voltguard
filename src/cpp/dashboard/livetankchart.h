
#ifndef LIVETANKCHART_H
#define LIVETANKCHART_H

#include <QWidget>
#include <QVector>
#include <QString>

class LiveTankChart : public QWidget
{
    Q_OBJECT

public:
    explicit LiveTankChart(QWidget *parent = nullptr);

    void addDataPoint(
    const QString &scenario,
    const QString &timestamp,
    int sequence,
    double level,
    double volume,
    double inflow,
    double outflow,
    bool pumpOn
    );

    void addEvent(
        const QString &timestamp,
        const QString &eventType,
        const QString &label,
        const QString &status,
        int sequence
    );

    void clearData();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct TelemetryPoint
    {
    QString timestamp;
    int sequence;
    double level;
    double volume;
    double inflow;
    double outflow;
    bool pumpOn;
    };

    struct TimelineEvent
    {
        QString timestamp;
        QString eventType;
        QString label;
        QString status;
        int sequence;
    };

    QVector<TelemetryPoint> normalFillingHistory;
    QVector<TelemetryPoint> pumpOffConsumptionHistory;
    QVector<TimelineEvent> eventHistory;

    static constexpr int MAX_NORMAL_POINTS = 20;
    static constexpr int MAX_PUMP_OFF_POINTS = 20;
    static constexpr int MAX_EVENTS = 180;
};

#endif // LIVETANKCHART_H