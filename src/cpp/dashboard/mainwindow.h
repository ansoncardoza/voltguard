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
class QTableWidget;
class LiveTankChart;
class LiveTrafficChart;

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

    // Live decision bridge process
    QProcess *liveDecisionProcess;
    QByteArray liveDecisionOutputBuffer;

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
    // Physics Monitoring live widgets
    QLabel *physicsTankValue;
    QLabel *physicsPumpValue;
    QLabel *physicsValveValue;
    QLabel *physicsFlowValue;

    QProgressBar *physicsTankLevelBar;
    QLabel *physicsTankVisualizationValue;

    QLabel *physicsSnapshotInfo;
    QLabel *physicsValidationSummary;
    // Live Physics Validation
    QLabel *livePhysicsValidationSummary;
    QLabel *livePhysicsViolationLabel;
    QLabel *livePhysicsWarningLabel;
    QLabel *physicsViolationLabel;
    QLabel *physicsWarningLabel;
    
    // Network Traffic live monitoring
    QLabel *trafficSummary;
    QTableWidget *trafficTable;
    QLabel *frameDetails;
    LiveTrafficChart *trafficChart;

    int liveTotalFrames = 0;
    int liveValidCount = 0;
    int liveValidationFailureCount = 0;
    int liveInvalidFrameCount = 0;

    // Per-scenario traffic history counters
    int normalLiveValidCount = 0;
    int normalLiveValidationFailureCount = 0;
    int normalLiveInvalidCount = 0;

    int pumpOffLiveValidCount = 0;
    int pumpOffLiveValidationFailureCount = 0;
    int pumpOffLiveInvalidCount = 0;

    int invalidTrafficValidCount = 0;
    int invalidTrafficValidationFailureCount = 0;
    int invalidTrafficInvalidCount = 0;
 
    // Protocol Analysis live monitoring
    QLabel *liveProtocolSummary;
    QLabel *liveProtocolFindingsSummary;
    QLabel *liveSecuritySummary;
    QLabel *liveDecisionSummary;
    QTableWidget *liveSecurityEventTable;
    int liveProtocolTotalFrames = 0;
    int liveProtocolFC03Count = 0;
    int liveProtocolFC06Count = 0;
    int liveProtocolUnsupportedCount = 0;
    int liveProtocolValidCount = 0;
    int liveProtocolValidationFailureCount = 0;
    int liveProtocolInvalidFrameCount = 0;

    // Live protocol validation findings
    int liveProtocolInvalidQuantityCount = 0;
    int liveProtocolInvalidLengthCount = 0;
    int liveProtocolInvalidUnitCount = 0;
    int liveProtocolRegisterRangeCount = 0;
    int liveProtocolInvalidMbapCount = 0;

    // Existing dashboard methods
    void createPages();
    void createNavigation();
    void switchPage(int pageIndex);

    // New live simulation methods
    void startLiveSimulation();
    void stopLiveSimulation();
    void handleLiveOutput();
    void handleLiveDecisionOutput();
    void processLiveSample(const QByteArray &jsonLine);
    void updateLiveConnectionStatus();
};

#endif // MAINWINDOW_H