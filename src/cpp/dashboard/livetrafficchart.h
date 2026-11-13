
#ifndef LIVETRAFFICCHART_H
#define LIVETRAFFICCHART_H

#include <QWidget>
#include <QVector>
#include <QString>

class LiveTrafficChart : public QWidget
{
    Q_OBJECT

public:
    explicit LiveTrafficChart(QWidget *parent = nullptr);

    void addDataPoint(
        const QString &scenario,
        int valid,
        int validationFailures,
        int invalid
    );

    void clearData();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct TrafficSnapshot
    {
        int valid;
        int validationFailures;
        int invalid;
    };

    QVector<TrafficSnapshot> normalFillingHistory;
    QVector<TrafficSnapshot> pumpOffConsumptionHistory;
    QVector<TrafficSnapshot> invalidTrafficHistory;

    static constexpr int MAX_POINTS = 60;
};

#endif // LIVETRAFFICCHART_H
