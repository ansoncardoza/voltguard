#ifndef LIVETANKCHART_H
#define LIVETANKCHART_H

#include <QWidget>
#include <QVector>

class LiveTankChart : public QWidget
{
    Q_OBJECT

public:
    explicit LiveTankChart(QWidget *parent = nullptr);

    void addDataPoint(double level);
    void clearData();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> levels;
    static constexpr int MAX_POINTS = 60;
};

#endif // LIVETANKCHART_H
