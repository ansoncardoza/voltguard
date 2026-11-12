
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QDateTime>

class QStackedWidget;
class QPushButton;
class QProcess;
class QLabel;
class QComboBox;
class QProgressBar;
class QTimer;
class LiveTankChart;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;

    // Existing offline analysis process
    QProcess *analysisProcess;

// New live simulation process
QProcess *liveSimulationProcess;
QTimer *liveDataWatchdog;
QByteArray liveOutputBuffer;
QDateTime lastLiveSampleAt;

    // Existing navigation buttons
    QPushButton *overviewButton;
    QPushButton *networkButton;
    QPushButton *protocolButton;
    QPushButton *physicsButton;
    QPushButton *alertsButton;
    QPushButton *reportsButton;

    // Live homepage controls
    QComboBox *liveScenarioSelector;
    QPushButton *startSimulationButton;
    QPushButton *stopSimulationButton;

    // Live homepage status
    QLabel *liveConnectionLabel;
    QLabel *liveScenarioLabel;
    QLabel *tankVolumeValueLabel;
    QLabel *tankLevelValueLabel;
    QLabel *pumpStatusValueLabel;
    QLabel *inletFlowValueLabel;
    QLabel *outletFlowValueLabel;
    QLabel *validationValueLabel;

    // Tank visualization
    QProgressBar *tankLevelProgress;
    LiveTankChart *tankLevelChart;

    // Existing dashboard methods
    void createPages();
    void createNavigation();
    void switchPage(int pageIndex);

    // New live simulation methods
    void startLiveSimulation();
    void stopLiveSimulation();
    void handleLiveOutput();
    void processLiveSample(const QByteArray &jsonLine);
    void updateLiveConnectionStatus();
};

#endif // MAINWINDOW_H