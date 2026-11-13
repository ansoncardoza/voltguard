#include "mainwindow.h"
#include "livetankchart.h"
#include "livetrafficchart.h"

#include <QProgressBar>
#include <QProcess>
#include <QComboBox>
#include <QDateTime>
#include <QJsonValue>
#include <QStringList>
#include <QJsonArray>
#include <QList>
#include <QTimer>
#include <QScrollArea>
#include <QStackedWidget>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QGridLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QComboBox>
#include <QStringList>
#include <QColor>
#include <QBrush>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(nullptr),
      analysisProcess(nullptr),
      liveSimulationProcess(nullptr),
      liveDecisionProcess(nullptr),
      liveDataWatchdog(nullptr),
      overviewButton(nullptr),
      networkButton(nullptr),
      protocolButton(nullptr),
      physicsButton(nullptr),
      alertsButton(nullptr),
      reportsButton(nullptr),
      liveScenarioSelector(nullptr),
      startSimulationButton(nullptr),
      stopSimulationButton(nullptr),
      liveConnectionLabel(nullptr),
      liveScenarioLabel(nullptr),
      tankVolumeValueLabel(nullptr),
      tankLevelValueLabel(nullptr),
      pumpStatusValueLabel(nullptr),
      inletFlowValueLabel(nullptr),
      outletFlowValueLabel(nullptr),
      validationValueLabel(nullptr),
      tankLevelProgress(nullptr),
      tankLevelChart(nullptr),
      physicsTankValue(nullptr),
      physicsPumpValue(nullptr),
      physicsValveValue(nullptr),
      physicsFlowValue(nullptr),
      physicsTankLevelBar(nullptr),
      physicsTankVisualizationValue(nullptr),
      physicsSnapshotInfo(nullptr),
      physicsValidationSummary(nullptr),
      physicsViolationLabel(nullptr),
      physicsWarningLabel(nullptr),
      livePhysicsValidationSummary(nullptr),
      livePhysicsViolationLabel(nullptr),
      livePhysicsWarningLabel(nullptr),
      trafficSummary(nullptr),
      trafficTable(nullptr),
      frameDetails(nullptr),
      liveProtocolSummary(nullptr),
      liveProtocolFindingsSummary(nullptr),
      liveSecuritySummary(nullptr),
      liveDecisionSummary(nullptr),
      liveSecurityEventTable(nullptr)
   {
    setWindowTitle("VoltGuard - ICS/SCADA Security Dashboard");
    resize(1100, 700);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #f1f5f9;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);


    // Sidebar
    QFrame *sidebar = new QFrame(centralWidget);
    sidebar->setFixedWidth(220);

    sidebar->setStyleSheet(
        "QFrame { background-color: #0f172a; }"
    );

    sidebar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    sidebar->setEnabled(true);
    sidebar->setMouseTracking(true);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

    QLabel *logo = new QLabel("VOLTGUARD");
    logo->setStyleSheet(
        "color: white; font-size: 22px; font-weight: bold;"
    );

    sidebarLayout->addWidget(logo);
    sidebarLayout->addSpacing(30);

    // Navigation buttons
    overviewButton = new QPushButton("System Overview");
    networkButton = new QPushButton("Network Traffic");
    protocolButton = new QPushButton("Protocol Analysis");
    physicsButton = new QPushButton("Physics Monitoring");
    alertsButton = new QPushButton("Security Alerts");
    reportsButton = new QPushButton("Reports");

    // Navigation buttons
    overviewButton = new QPushButton("System Overview");
    networkButton = new QPushButton("Network Traffic");
    protocolButton = new QPushButton("Protocol Analysis");
    physicsButton = new QPushButton("Physics Monitoring");
    alertsButton = new QPushButton("Security Alerts");
    reportsButton = new QPushButton("Reports");

    QList<QPushButton *> buttons = {
        overviewButton,
        networkButton,
        protocolButton,
        physicsButton,
        alertsButton,
        reportsButton
    };


for (QPushButton *button : buttons)
{
    button->setEnabled(true);
    button->setVisible(true);
    button->setFocusPolicy(Qt::StrongFocus);
    button->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    button->setMinimumHeight(45);
    button->setCursor(Qt::PointingHandCursor);

    button->setStyleSheet(
        "QPushButton {"
        "color: white;"
        "background-color: #1e293b;"
        "border: 1px solid #334155;"
        "text-align: left;"
        "padding: 12px;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "background-color: #2563eb;"
        "color: white;"
        "}"
        "QPushButton:pressed {"
        "background-color: #1d4ed8;"
        "}"
    );

    sidebarLayout->addWidget(button);
}

sidebarLayout->addStretch();

// Stacked pages with vertical scrolling
QScrollArea *contentScrollArea = new QScrollArea(centralWidget);

contentScrollArea->setWidgetResizable(true);
contentScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
contentScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
contentScrollArea->setFrameShape(QFrame::NoFrame);

stackedWidget = new QStackedWidget();

stackedWidget->setMinimumWidth(500);
stackedWidget->setStyleSheet(
    "QStackedWidget { background-color: #f1f5f9; }"
);

contentScrollArea->setWidget(stackedWidget);

    analysisProcess = new QProcess(this);
    // Independent live simulation process
    liveSimulationProcess = new QProcess(this);
    // Independent live decision bridge process
liveDecisionProcess = new QProcess(this);

// Receive live decisions from the Python bridge
connect(
    liveDecisionProcess,
    &QProcess::readyReadStandardOutput,
    this,
    &MainWindow::handleLiveDecisionOutput
);

// Monitor decision bridge errors independently
connect(
    liveDecisionProcess,
    &QProcess::errorOccurred,
    this,
    [this](QProcess::ProcessError error)
    {
        Q_UNUSED(error);

        liveDecisionSummary->setText(
            "LIVE SIMULATED DECISION: BRIDGE ERROR\n\n" +
            liveDecisionProcess->errorString()
        );
    }
);
    // Receive continuous JSON Lines from Python
connect(
    liveSimulationProcess,
    &QProcess::readyReadStandardOutput,
    this,
    &MainWindow::handleLiveOutput
);

// Monitor Python process errors
connect(
    liveSimulationProcess,
    &QProcess::errorOccurred,
    this,
    [this](QProcess::ProcessError error)
    {
        Q_UNUSED(error);
        liveConnectionLabel->setText(
            "Simulation Error: " +
            liveSimulationProcess->errorString()
        );
    }
);

    // Detect interrupted or stale live data
    liveDataWatchdog = new QTimer(this);
    liveDataWatchdog->setInterval(1000);
    connect(
    liveDataWatchdog,
    &QTimer::timeout,
    this,
    &MainWindow::updateLiveConnectionStatus
);

    createPages();

// Live simulation button connections
connect(
    startSimulationButton,
    &QPushButton::clicked,
    this,
    &MainWindow::startLiveSimulation
);

connect(
    stopSimulationButton,
    &QPushButton::clicked,
    this,
    &MainWindow::stopLiveSimulation
);

    // Navigation button connections
    connect(overviewButton, &QPushButton::clicked,
            this, [this]() { switchPage(0); });

    connect(networkButton, &QPushButton::clicked,
            this, [this]() { switchPage(1); });

    connect(protocolButton, &QPushButton::clicked,
            this, [this]() { switchPage(2); });

    connect(physicsButton, &QPushButton::clicked,
            this, [this]() { switchPage(3); });

    connect(alertsButton, &QPushButton::clicked,
            this, [this]() { switchPage(4); });

    connect(reportsButton, &QPushButton::clicked,
            this, [this]() { switchPage(5); });

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(contentScrollArea, 1);

    switchPage(0);
}

MainWindow::~MainWindow()
{
    stopLiveSimulation();
}

// Create dashboard pages

void MainWindow::createPages()
{

    // =========================================
    // PAGE 1: SYSTEM OVERVIEW - LIVE MONITORING
    // =========================================

    QWidget *overviewPage = new QWidget(stackedWidget);
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewPage);

    overviewLayout->setSpacing(18);
    overviewLayout->setContentsMargins(25, 25, 25, 25);

    // -----------------------------------------
    // Page Heading
    // -----------------------------------------

    QLabel *overviewTitle = new QLabel("System Overview");

    overviewTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background: transparent;"
    );

    QLabel *overviewSubtitle = new QLabel(
        "Live Physics-Aware ICS/SCADA Process Monitoring"
    );

    overviewSubtitle->setStyleSheet(
        "color: #64748b;"
        "font-size: 14px;"
        "background: transparent;"
    );

    overviewLayout->addWidget(overviewTitle);
    overviewLayout->addWidget(overviewSubtitle);

    // -----------------------------------------
    // Simulation Controls
    // -----------------------------------------

    QFrame *controlPanel = new QFrame();

    controlPanel->setStyleSheet(
        "QFrame {"
        "background: white;"
        "border: 1px solid #020f20;"
        "border-radius: 10px;"
        "}"
    );

    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);

    QLabel *modeLabel = new QLabel("Simulation Scenario:");

    liveScenarioSelector = new QComboBox();

    liveScenarioSelector->addItem(
    "Normal Tank Filling",
    "normal_filling"
    );

    liveScenarioSelector->addItem(
    "Pump OFF - Normal Water Consumption",
    "pump_off_consumption"
    );

    liveScenarioSelector->addItem(
    "Invalid Traffic Test",
    "invalid_traffic"
    );

    liveScenarioSelector->setMinimumWidth(250);
    liveScenarioSelector->setMinimumHeight(38);

    liveScenarioSelector->setStyleSheet(
    "QComboBox {"
    "background-color: #ffffff;"
    "color: #0f172a;"
    "border: 1px solid #cbd5e1;"
    "border-radius: 6px;"
    "padding: 6px 10px;"
    "font-size: 13px;"
    "min-width: 250px;"
    "}"
    "QComboBox:hover {"
    "border: 1px solid #2563eb;"
    "}"
    "QComboBox::drop-down {"
    "width: 30px;"
    "border: none;"
    "background-color: #f1f5f9;"
    "}"
    "QComboBox QAbstractItemView {"
    "background-color: #ffffff;"
    "color: #0f172a;"
    "selection-background-color: #506581;"
    "selection-color: #0f172a;"
    "border: 1px solid #cbd5e1;"
    "outline: none;"
    "}"
    );

    startSimulationButton = new QPushButton("Start Simulation");
    stopSimulationButton = new QPushButton("Stop Simulation");

    startSimulationButton->setStyleSheet(
        "QPushButton {"
        "background: #2563eb;"
        "color: white;"
        "padding: 10px 16px;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover { background: #1d4ed8; }"
    );

    stopSimulationButton->setStyleSheet(
        "QPushButton {"
        "background: #dc2626;"
        "color: white;"
        "padding: 10px 16px;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover { background: #b91c1c; }"
    );

    stopSimulationButton->setEnabled(false);

    controlLayout->addWidget(modeLabel);
    controlLayout->addWidget(liveScenarioSelector);
    controlLayout->addStretch();
    controlLayout->addWidget(startSimulationButton);
    controlLayout->addWidget(stopSimulationButton);

    overviewLayout->addWidget(controlPanel);

    // -----------------------------------------
    // Live Connection Status
    // -----------------------------------------

    liveConnectionLabel = new QLabel(
        "● DISCONNECTED - Simulation not started"
    );

    liveConnectionLabel->setStyleSheet(
        "background: #f1f5f9;"
        "color: #f10505;"
        "padding: 12px;"
        "border-radius: 6px;"
        "font-weight: bold;"
    );

    overviewLayout->addWidget(liveConnectionLabel);

    liveScenarioLabel = new QLabel(
    "Selected Scenario: " + liveScenarioSelector->currentText()
    );

    liveScenarioLabel->setStyleSheet(
        "color: #334155;"
        "font-size: 13px;"
        "background: transparent;"
    );

    overviewLayout->addWidget(liveScenarioLabel);
    // Keep the displayed scenario synchronized with the selector.
connect(
    liveScenarioSelector,
    &QComboBox::currentTextChanged,
    this,
    [this](const QString &scenarioName)
    {
        liveScenarioLabel->setText(
            "Selected Scenario: " + scenarioName
        );

        // The pump remains OFF until a live sample confirms its state.
        if (liveSimulationProcess->state() == QProcess::NotRunning)
        {
            pumpStatusValueLabel->setText("OFF");
        }
    }
);

    // -----------------------------------------
    // Live Metric Cards
    // -----------------------------------------

    QGridLayout *metricsGrid = new QGridLayout();

    metricsGrid->setSpacing(15);

    auto createLiveCard = [](
        const QString &title,
        const QString &initialValue,
        const QString &description,
        const QString &color,
        QLabel **valueOutput
    )
    {
        QFrame *card = new QFrame();

        card->setMinimumHeight(80);

        card->setStyleSheet(
            "QFrame {"
            "background: white;"
            "border: 1px solid #e2e8f0;"
            "border-radius: 10px;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(4);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(3);

        QLabel *titleLabel = new QLabel(title);
        QLabel *valueLabel = new QLabel(initialValue);
        QLabel *descriptionLabel = new QLabel(description);

        titleLabel->setStyleSheet(
            "color: #64748b;"
            "border: none;"
            "background: transparent;"
        );

        valueLabel->setStyleSheet(
            "color: " + color +
            "; font-size: 21px;"
            "; font-weight: bold;"
            "; border: none;"
            "; background: transparent;"
        );

        descriptionLabel->setStyleSheet(
            "color: #94a3b8;"
            "font-size: 11px;"
            "border: none;"
            "background: transparent;"
        );

        *valueOutput = valueLabel;

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(descriptionLabel);
        layout->addStretch();

        return card;
    };

    metricsGrid->addWidget(
        createLiveCard(
            "Tank Volume",
            "-- L",
            "Current simulated water volume",
            "#2563eb",
            &tankVolumeValueLabel
        ),
        0, 0
    );

    metricsGrid->addWidget(
        createLiveCard(
            "Tank Level",
            "-- %",
            "Current tank capacity utilization",
            "#0891b2",
            &tankLevelValueLabel
        ),
        0, 1
    );

    metricsGrid->addWidget(
        createLiveCard(
            "Pump Status",
            "OFF",
            "Current simulated pump state",
            "#dc2626",
            &pumpStatusValueLabel
        ),
        0, 2
    );

    metricsGrid->addWidget(
        createLiveCard(
            "Inlet Flow",
            "-- L/min",
            "Water entering the tank",
            "#16a34a",
            &inletFlowValueLabel
        ),
        1, 0
    );

    metricsGrid->addWidget(
        createLiveCard(
            "Outlet Flow",
            "-- L/min",
            "Normal water consumption",
            "#d97706",
            &outletFlowValueLabel
        ),
        1, 1
    );

    metricsGrid->addWidget(
        createLiveCard(
        "Physics Validation",
        "NOT RUN",
        "Operating-condition validation",
        "#dc2626",
    &validationValueLabel
),
        1, 2
    );

    overviewLayout->addLayout(metricsGrid);

    // -----------------------------------------
    // Tank Level Progress Visualization
    // -----------------------------------------

    QFrame *tankPanel = new QFrame();

    tankPanel->setStyleSheet(
        "QFrame {"
        "background: white;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 10px;"
        "}"
    );

    QVBoxLayout *tankLayout = new QVBoxLayout(tankPanel);

    QLabel *tankTitle = new QLabel("Tank Level Monitoring");

    tankTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "border: none;"
        "background: transparent;"
    );

    tankLevelProgress = new QProgressBar();
    tankLevelProgress->setRange(0, 100);
    tankLevelProgress->setValue(0);
    tankLevelProgress->setFormat("Waiting for live data...");
    tankLevelProgress->setMinimumHeight(25);

    tankLevelProgress->setStyleSheet(
        "QProgressBar {"
        "border: 1px solid #cbd5e1;"
        "border-radius: 6px;"
        "background: #f1f5f9;"
        "text-align: center;"
        "color: #0f172a;"
        "}"
        "QProgressBar::chunk {"
        "background: #2563eb;"
        "border-radius: 5px;"
        "}"
    );

    tankLayout->addWidget(tankTitle);
    tankLayout->addWidget(tankLevelProgress);

    overviewLayout->addWidget(tankPanel);

    // -----------------------------------------
    // Live Tank Level History Graph
    // -----------------------------------------

    QFrame *chartPanel = new QFrame();

    chartPanel->setStyleSheet(
        "QFrame {"
        "background: white;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 10px;"
        "}"
    );

    QVBoxLayout *chartLayout = new QVBoxLayout(chartPanel);

    QLabel *chartTitle = new QLabel(
        "Live Tank Level History"
    );

    chartTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "border: none;"
        "background: transparent;"
    );

    tankLevelChart = new LiveTankChart();

    chartLayout->addWidget(chartTitle);
    chartLayout->addWidget(tankLevelChart);

    overviewLayout->addWidget(chartPanel);

    // -----------------------------------------
    // Add Homepage
    // -----------------------------------------

    overviewLayout->addStretch();
    stackedWidget->addWidget(overviewPage);

    // =========================================
    // PAGE 2: NETWORK TRAFFIC
    // =========================================

    QWidget *networkPage = new QWidget(stackedWidget);
    QVBoxLayout *networkLayout = new QVBoxLayout(networkPage);

    QLabel *networkTitle = new QLabel("Network Traffic");

    networkTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *networkSubtitle = new QLabel(
        "Synthetic Modbus/TCP Traffic Monitoring | LOCAL_TEST"
    );

    networkSubtitle->setStyleSheet(
        "color: #64748b;"
        "font-size: 14px;"
        "background-color: transparent;"
    );

    trafficSummary = new QLabel(
        "Loading traffic statistics..."
    );

    trafficSummary->setStyleSheet(
        "font-size: 15px;"
        "font-weight: bold;"
        "color: #334155;"
        "background-color: transparent;"
    );

    // Live Modbus Traffic History Chart
    trafficChart = new LiveTrafficChart(networkPage);

    trafficTable = new QTableWidget();

    trafficTable->setColumnCount(5);

    trafficTable->setHorizontalHeaderLabels({
        "Frame",
        "Transaction ID",
        "Function Code",
        "Status",
        "Validation Result"
    });

    trafficTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    trafficTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );

    trafficTable->setSelectionMode(
        QAbstractItemView::SingleSelection
    );

    trafficTable->verticalHeader()->setVisible(false);

    trafficTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents
    );

    trafficTable->horizontalHeader()->setStretchLastSection(true);

    trafficTable->setMinimumHeight(180);
    trafficTable->setShowGrid(true);
    trafficTable->setAlternatingRowColors(true);

    trafficTable->setStyleSheet(
    "QTableWidget {"
    "background-color: #1e293b;"
    "color: #f1f5f9;"
    "gridline-color: #475569;"
    "border: 1px solid #475569;"
    "font-size: 13px;"
    "}"
    "QTableWidget::item {"
    "padding: 6px;"
    "}"
    "QTableWidget::item:selected {"
    "background-color: #2563eb;"
    "color: #ffffff;"
    "}"
    "QHeaderView::section {"
    "background-color: #334155;"
    "color: #ffffff;"
    "padding: 8px;"
    "border: 1px solid #475569;"
    "font-weight: bold;"
    "}"
     );

    QLabel *frameDetailsTitle = new QLabel("Selected Frame Details");

    frameDetailsTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    frameDetails = new QLabel(
        "Select a traffic frame to inspect its parsed information."
    );

    frameDetails->setWordWrap(true);

    frameDetails->setStyleSheet(
        "color: #334155;"
        "font-size: 14px;"
        "background-color: transparent;"
    );

    networkLayout->addWidget(networkTitle);
    networkLayout->addWidget(networkSubtitle);
    networkLayout->addWidget(trafficSummary);
    networkLayout->addWidget(trafficChart);
    networkLayout->addWidget(trafficTable, 1);
    networkLayout->addWidget(frameDetailsTitle);
    networkLayout->addWidget(frameDetails);

    // Load C++ parser results

    QString trafficReportPath = QDir(
        QCoreApplication::applicationDirPath()
    ).absoluteFilePath("../data/generated/parser_results.json");

    trafficReportPath = QDir::cleanPath(trafficReportPath);

    QFile trafficFile(trafficReportPath);

    if (!trafficFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        trafficSummary->setText(
            "Traffic Analysis: REPORT NOT FOUND"
        );
    }
    else
    {
        QJsonParseError trafficParseError;

        QJsonDocument trafficDocument = QJsonDocument::fromJson(
            trafficFile.readAll(),
            &trafficParseError
        );

        trafficFile.close();

        if (trafficParseError.error != QJsonParseError::NoError ||
            !trafficDocument.isObject())
        {
            trafficSummary->setText(
                "Traffic Analysis: INVALID REPORT"
            );
        }
        else
        {
            QJsonObject trafficObject = trafficDocument.object();

            QJsonArray events = trafficObject.value("events").toArray();

            int validCount = 0;
            int validationFailureCount = 0;
            int invalidFrameCount = 0;

            trafficTable->setRowCount(events.size());

            for (int i = 0; i < events.size(); ++i)
            {
                QJsonObject event = events.at(i).toObject();

                QString status = event.value("status").toString();

                if (status == "VALID")
                {
                    ++validCount;
                }
                else if (status == "VALIDATION_FAILED")
                {
                    ++validationFailureCount;
                }
                else if (status == "INVALID_FRAME")
                {
                    ++invalidFrameCount;
                }

                QString frameNumber =
                    QString::number(event.value("frame_number").toInt());

                QString transactionId = event.contains("transaction_id")
                    ? QString::number(
                        event.value("transaction_id").toInt()
                    )
                    : "N/A";

                QString functionCode = event.contains("function_code")
                    ? QString("0x%1").arg(
                        event.value("function_code").toInt(),
                        2,
                        16,
                        QChar('0')
                    ).toUpper()
                    : "N/A";

                QString reason = event.value("reason").toString();

                trafficTable->setItem(
                    i, 0, new QTableWidgetItem(frameNumber)
                );

                trafficTable->setItem(
                    i, 1, new QTableWidgetItem(transactionId)
                );

                trafficTable->setItem(
                    i, 2, new QTableWidgetItem(functionCode)
                );

                trafficTable->setItem(
                    i, 3, new QTableWidgetItem(status)
                );

                trafficTable->setItem(
                    i, 4, new QTableWidgetItem(reason)
                );

                trafficTable->item(i, 3)->setData(
    Qt::UserRole,
    QJsonDocument(event).toJson(QJsonDocument::Compact)
);

// Apply status-based row highlighting
QColor rowColor;
QColor textColor("#f1f5f9");

if (status == "VALID")
{
    rowColor = QColor("#14532d");
}
else if (status == "VALIDATION_FAILED")
{
    rowColor = QColor("#78350f");
}
else if (status == "INVALID_FRAME")
{
    rowColor = QColor("#7f1d1d");
}
else
{
    rowColor = QColor("#334155");
}

// Apply colors to all five columns
for (int column = 0; column < trafficTable->columnCount(); ++column)
{
    QTableWidgetItem *cell = trafficTable->item(i, column);

    if (cell)
    {
        cell->setBackground(QBrush(rowColor));
        cell->setForeground(QBrush(textColor));
    }
}
} // End of event-population loop

            trafficSummary->setText(
                QString("Total Frames: %1    |    Valid: %2    |    "
                        "Validation Failures: %3    |    Invalid Frames: %4")
                    .arg(events.size())
                    .arg(validCount)
                    .arg(validationFailureCount)
                    .arg(invalidFrameCount)
            );
        }
    }

    // Display details when a frame is selected

    connect(
        trafficTable,
        &QTableWidget::cellClicked,
        this,
        [this](int row, int column)
        {
            Q_UNUSED(column);

            QTableWidgetItem *item = trafficTable->item(row, 3);

            if (!item)
                return;

            QByteArray eventData =
                item->data(Qt::UserRole).toByteArray();

            QJsonDocument document =
                QJsonDocument::fromJson(eventData);

            QJsonObject event = document.object();

            QString details;

            details += "Frame Number: " +
                QString::number(event.value("frame_number").toInt()) + "\n";

            details += "Status: " +
                event.value("status").toString() + "\n";

            details += "Reason: " +
                event.value("reason").toString() + "\n";

            details += "Transaction ID: " +
                (event.contains("transaction_id")
                    ? QString::number(
                        event.value("transaction_id").toInt()
                    )
                    : "N/A") + "\n";

            details += "Unit ID: " +
                (event.contains("unit_id")
                    ? QString::number(
                        event.value("unit_id").toInt()
                    )
                    : "N/A") + "\n";

            details += "Function Code: " +
                (event.contains("function_code")
                    ? QString("0x%1").arg(
                        event.value("function_code").toInt(),
                        2,
                        16,
                        QChar('0')
                    ).toUpper()
                    : "N/A") + "\n";

            details += "Start Address: " +
                (event.contains("start_address")
                    ? QString::number(
                        event.value("start_address").toInt()
                    )
                    : "N/A") + "\n";

            details += "Quantity: " +
    (event.contains("quantity")
        ? QString::number(
            event.value("quantity").toInt()
        )
        : "N/A") + "\n\n";

    // Function Code 0x06: Write Single Register details
        if (event.contains("register_address") &&
            event.contains("register_value"))
    {
    details += "\nRegister Address: " +
        QString::number(
            event.value("register_address").toInt()
        ) + "\n";

    details += "Register Write Value: " +
        QString::number(
            event.value("register_value").toInt()
        ) + "\n";
    }

        // Raw hexadecimal frame
        details += "Raw Hexadecimal Frame:\n";

    details += event.contains("raw_hex")
    ? event.value("raw_hex").toString()
    : "Raw frame data unavailable.";

    frameDetails->setText(details);
        }
    );

    stackedWidget->addWidget(networkPage);

// =========================================
// PAGE 3: PROTOCOL ANALYSIS
// =========================================

QWidget *protocolPage = new QWidget(stackedWidget);
QVBoxLayout *protocolLayout = new QVBoxLayout(protocolPage);

QLabel *protocolTitle = new QLabel("Protocol Analysis");

protocolTitle->setStyleSheet(
    "font-size: 26px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
);

QLabel *protocolSubtitle = new QLabel(
    "Modbus/TCP Function Code and Protocol Validation Analysis"
);

protocolSubtitle->setStyleSheet(
    "color: #64748b;"
    "font-size: 14px;"
    "background-color: transparent;"
);

// Protocol statistics
QLabel *totalProtocolFrames = new QLabel("Total Frames: --");
QLabel *fc03Frames = new QLabel("FC03 Frames: --");
QLabel *fc06Frames = new QLabel("FC06 Frames: --");
QLabel *unsupportedFunctions = new QLabel("Unsupported Functions: --");

QString protocolCardStyle =
    "background-color: #1e293b;"
    "color: #f1f5f9;"
    "font-size: 15px;"
    "font-weight: bold;"
    "padding: 18px;"
    "border-radius: 8px;";

totalProtocolFrames->setStyleSheet(protocolCardStyle);
fc03Frames->setStyleSheet(protocolCardStyle);
fc06Frames->setStyleSheet(protocolCardStyle);
unsupportedFunctions->setStyleSheet(protocolCardStyle);

QHBoxLayout *protocolStatsLayout = new QHBoxLayout();

protocolStatsLayout->addWidget(totalProtocolFrames);
protocolStatsLayout->addWidget(fc03Frames);
protocolStatsLayout->addWidget(fc06Frames);
protocolStatsLayout->addWidget(unsupportedFunctions);

QLabel *functionChartTitle = new QLabel(
    "Function Code Distribution"
);

functionChartTitle->setStyleSheet(
    "font-size: 18px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
);

QWidget *functionChart = new QWidget(protocolPage);
functionChart->setMinimumHeight(180);

QVBoxLayout *functionChartLayout =
    new QVBoxLayout(functionChart);

functionChartLayout->setSpacing(12);

QLabel *protocolStatusInfo = new QLabel(
    "Loading protocol analysis..."
);

// Live Protocol Analysis panel
liveProtocolSummary = new QLabel(
    "LIVE PROTOCOL ANALYSIS\n\n"
    "Live Frames: 0\n"
    "FC03 Frames: 0\n"
    "FC06 Frames: 0\n"
    "Unsupported Functions: 0\n"
    "Valid Frames: 0\n"
    "Validation Failures: 0\n"
    "Invalid Frames: 0"
);

liveProtocolSummary->setWordWrap(true);

liveProtocolSummary->setStyleSheet(
    "background-color: #e0f2fe;"
    "color: #0c4a6e;"
    "font-size: 14px;"
    "font-weight: bold;"
    "padding: 16px;"
    "border-radius: 8px;"
);

// Live Protocol Validation Findings panel
liveProtocolFindingsSummary = new QLabel(
    "LIVE VALIDATION FINDINGS\n\n"
    "Unsupported Function Code: 0\n"
    "Invalid Register Quantity: 0\n"
    "Invalid Data Length: 0\n"
    "Invalid Unit ID: 0\n"
    "Register Range Exceeded: 0\n"
    "Invalid MBAP Frame: 0"
);

liveProtocolFindingsSummary->setWordWrap(true);

liveProtocolFindingsSummary->setStyleSheet(
    "background-color: #fef3c7;"
    "color: #92400e;"
    "font-size: 14px;"
    "font-weight: bold;"
    "padding: 16px;"
    "border-radius: 8px;"
);

protocolStatusInfo->setWordWrap(true);

protocolStatusInfo->setStyleSheet(
    "color: #334155;"
    "font-size: 14px;"
    "background-color: transparent;"
);

// Add widgets to page
protocolLayout->addWidget(protocolTitle);
protocolLayout->addWidget(protocolSubtitle);
protocolLayout->addLayout(protocolStatsLayout);

// Side-by-side live protocol panels
QHBoxLayout *liveProtocolLayout = new QHBoxLayout();
liveProtocolLayout->setSpacing(15);

liveProtocolLayout->addWidget(liveProtocolSummary, 1);
liveProtocolLayout->addWidget(liveProtocolFindingsSummary, 1);

protocolLayout->addLayout(liveProtocolLayout);

protocolLayout->addWidget(functionChartTitle);
protocolLayout->addWidget(functionChart);

protocolLayout->addWidget(protocolStatusInfo);
protocolLayout->addStretch();

// Load C++ parser results
QString protocolReportPath = QDir(
    QCoreApplication::applicationDirPath()
).absoluteFilePath("../data/generated/parser_results.json");

protocolReportPath = QDir::cleanPath(protocolReportPath);

QFile protocolFile(protocolReportPath);

if (!protocolFile.open(QIODevice::ReadOnly | QIODevice::Text))
{
    protocolStatusInfo->setText(
        "Protocol Analysis: REPORT NOT FOUND"
    );
}
else
{
    QJsonParseError protocolParseError;

    QJsonDocument protocolDocument = QJsonDocument::fromJson(
        protocolFile.readAll(),
        &protocolParseError
    );

    protocolFile.close();

    if (protocolParseError.error != QJsonParseError::NoError ||
        !protocolDocument.isObject())
    {
        protocolStatusInfo->setText(
            "Protocol Analysis: INVALID REPORT"
        );
    }
    else
    {
        QJsonObject protocolObject = protocolDocument.object();

        QJsonArray protocolEvents =
            protocolObject.value("events").toArray();

        int fc03Count = 0;
        int fc06Count = 0;
        int unsupportedCount = 0;
        int invalidQuantityCount = 0;
        int invalidLengthCount = 0;
        int invalidUnitCount = 0;
        int registerRangeCount = 0;
        int invalidMbapCount = 0;

        for (const QJsonValue &value : protocolEvents)
        {
            QJsonObject event = value.toObject();

            int functionCode =
                event.value("function_code").toInt(-1);

            QString status =
                event.value("status").toString();

            QString reason =
                event.value("reason").toString();

                if (reason.contains("INVALID REGISTER QUANTITY"))
    ++invalidQuantityCount;
     else if (reason.contains("INVALID DATA LENGTH"))
    ++invalidLengthCount;
     else if (reason == "INVALID UNIT ID")
    ++invalidUnitCount;
     else if (reason.contains("REGISTER RANGE"))
    ++registerRangeCount;
     else if (reason.contains("MBAP"))
    ++invalidMbapCount;

            if (functionCode == 3)
                ++fc03Count;

            if (functionCode == 6)
                ++fc06Count;

            if (reason == "UNSUPPORTED FUNCTION CODE")
                ++unsupportedCount;
        }

        totalProtocolFrames->setText(
            QString("Total Frames: %1").arg(protocolEvents.size())
        );

        fc03Frames->setText(
            QString("FC03 Frames: %1").arg(fc03Count)
        );

        fc06Frames->setText(
            QString("FC06 Frames: %1").arg(fc06Count)
        );

        unsupportedFunctions->setText(
            QString("Unsupported Functions: %1").arg(unsupportedCount)
        );


// Create a reusable function-code chart bar
auto addFunctionBar = [&](const QString &label,
                          int count,
                          const QString &color)
{
    QLabel *barLabel = new QLabel(
        QString("%1 (%2)").arg(label).arg(count)
    );

    barLabel->setStyleSheet(
        "color: #334155;"
        "font-size: 13px;"
        "font-weight: bold;"
        "background-color: transparent;"
    );

    QProgressBar *bar = new QProgressBar();

    bar->setRange(0, qMax(1, protocolEvents.size()));
    bar->setValue(count);
    bar->setTextVisible(false);
    bar->setFixedHeight(18);

    bar->setStyleSheet(
        "QProgressBar {"
        "background-color: #e2e8f0;"
        "border: none;"
        "border-radius: 8px;"
        "}"
        "QProgressBar::chunk {"
        "background-color: " + color + ";"
        "border-radius: 8px;"
        "}"
    );

    functionChartLayout->addWidget(barLabel);
    functionChartLayout->addWidget(bar);
};

// Add function-code distribution bars
addFunctionBar(
    "FC03 - Read Holding Registers",
    fc03Count,
    "#2563eb"
);

addFunctionBar(
    "FC06 - Write Single Register",
    fc06Count,
    "#16a34a"
);

addFunctionBar(
    "Unsupported Function Codes",
    unsupportedCount,
    "#dc2626"
);

// Validation findings chart
QLabel *validationTitle = new QLabel(
    "Protocol Validation Findings"
);

validationTitle->setStyleSheet(
    "font-size: 18px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
);

functionChartLayout->addWidget(validationTitle);

addFunctionBar("Unsupported Function Code", unsupportedCount, "#dc2626");
addFunctionBar("Invalid Register Quantity", invalidQuantityCount, "#ea580c");
addFunctionBar("Invalid Data Length", invalidLengthCount, "#d97706");
addFunctionBar("Invalid Unit ID", invalidUnitCount, "#ca8a04");
addFunctionBar("Register Range Exceeded", registerRangeCount, "#9333ea");
addFunctionBar("Invalid MBAP Frame", invalidMbapCount, "#64748b");

protocolStatusInfo->setText("");
    }
}

stackedWidget->addWidget(protocolPage);

    // =========================================
    // PAGE 4: PHYSICS MONITORING
    // =========================================

    QWidget *physicsPage = new QWidget(stackedWidget);
    QVBoxLayout *physicsLayout = new QVBoxLayout(physicsPage);
    physicsLayout->setSpacing(4);
    physicsLayout->setContentsMargins(15, 5, 15, 5);
    physicsLayout->setAlignment(Qt::AlignTop);

    QLabel *physicsTitle = new QLabel("Physics Monitoring");
physicsTitle->setContentsMargins(0, 0, 0, 0);

physicsTitle->setStyleSheet(
    "font-size: 23px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
    "padding: 0px;"
    "margin: 0px;"
);

physicsTitle->setSizePolicy(
    QSizePolicy::Preferred,
    QSizePolicy::Fixed
);

    QLabel *physicsSubtitle = new QLabel(
    "Synthetic Water Tank Process Monitoring"
);
physicsSubtitle->setContentsMargins(0, 0, 0, 0);

physicsSubtitle->setStyleSheet(
    "color: #64748b;"
    "font-size: 12px;"
    "background-color: transparent;"
    "padding: 0px;"
    "margin: 0px;"
);

physicsSubtitle->setSizePolicy(
    QSizePolicy::Preferred,
    QSizePolicy::Fixed
);

// Scenario Selector

QLabel *scenarioLabel = new QLabel("Historical Snapshot:");

QComboBox *scenarioSelector = new QComboBox();
scenarioSelector->addItem("Normal Snapshot", "process_snapshot.json");
scenarioSelector->addItem("Abnormal Snapshot", "abnormal_process_snapshot.json");
scenarioSelector->setMinimumWidth(250);

    scenarioSelector->setStyleSheet(
    "QComboBox {"
    "    background-color: white;"
    "    color: #1e293b;"
    "    border: 1px solid #cbd5e1;"
    "    border-radius: 6px;"
    "    padding: 8px;"
    "    min-width: 180px;"
    "}"
    "QComboBox::drop-down {"
    "    border: none;"
    "    width: 25px;"
    "}"
    "QComboBox QAbstractItemView {"
    "    background-color: white;"
    "    color: #1e293b;"
    "    selection-background-color: #2563eb;"
    "    selection-color: white;"
    "    border: 1px solid #cbd5e1;"
    "}"
    );

    QHBoxLayout *scenarioLayout = new QHBoxLayout();
    scenarioLayout->addWidget(scenarioLabel);
    scenarioLayout->addWidget(scenarioSelector);
    scenarioLayout->addStretch();

    // Process Variable Cards

    QGridLayout *physicsGrid = new QGridLayout();
    physicsGrid->setHorizontalSpacing(8);
    physicsGrid->setVerticalSpacing(0);
    physicsGrid->setContentsMargins(0, 0, 0, 0);

    auto createPhysicsCard = [](
        const QString &title,
        const QString &color
    )
    {
        QFrame *card = new QFrame();

        card->setFixedHeight(70);

        card->setStyleSheet(
            "QFrame {"
            "background-color: white;"
            "border: 1px solid #e2e8f0;"
            "border-radius: 10px;"
            "}"
        );

  QVBoxLayout *layout = new QVBoxLayout(card);
layout->setContentsMargins(12, 8, 12, 8);
layout->setSpacing(2);

        QLabel *titleLabel = new QLabel(title);
        QLabel *valueLabel = new QLabel("N/A");

       titleLabel->setStyleSheet(
    "color: #64748b;"
    "font-size: 12px;"
    "font-weight: 600;"
    "border: none;"
    "background-color: transparent;"
);

        valueLabel->setStyleSheet(
    "color: " + color +
    "; font-size: 21px;"
    "font-weight: bold;"
    "border: none;"
    "background-color: transparent;"
);

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addStretch();

        card->setProperty("valueLabel", QVariant::fromValue<QObject *>(valueLabel));

        return card;
    };

    QFrame *tankCard = createPhysicsCard("Tank Level", "#2563eb");
    QFrame *pumpCard = createPhysicsCard("Pump Status", "#16a34a");
    QFrame *valveCard = createPhysicsCard("Valve Opening", "#7c3aed");
    QFrame *flowCard = createPhysicsCard("Flow Rate", "#0891b2");

    physicsTankValue = qobject_cast<QLabel *>(
    tankCard->property("valueLabel").value<QObject *>()
);

    physicsPumpValue = qobject_cast<QLabel *>(
    pumpCard->property("valueLabel").value<QObject *>()
);

    physicsValveValue = qobject_cast<QLabel *>(
    valveCard->property("valueLabel").value<QObject *>()
);

    physicsFlowValue = qobject_cast<QLabel *>(
    flowCard->property("valueLabel").value<QObject *>()
);

// Keep local aliases for the existing snapshot loader.
QLabel *tankValue = physicsTankValue;
QLabel *pumpValue = physicsPumpValue;

// Initial state before live simulation starts.
pumpValue->setText("OFF");
pumpValue->setStyleSheet(
    "color: #dc2626;"
    "font-size: 25px;"
    "font-weight: bold;"
    "background-color: transparent;"
);

QLabel *valveValue = physicsValveValue;
QLabel *flowValue = physicsFlowValue;

physicsGrid->addWidget(pumpCard, 0, 0);
physicsGrid->addWidget(tankCard, 0, 1);
physicsGrid->addWidget(flowCard, 1, 0);
physicsGrid->addWidget(valveCard, 1, 1);

for (int column = 0; column < 2; ++column)
{
    physicsGrid->setColumnStretch(column, 1);
}

    // Tank Level Visualization

    QFrame *tankVisualizationCard = new QFrame();

    tankVisualizationCard->setMinimumSize(170, 145);
    tankVisualizationCard->setMaximumSize(210, 155);

    tankVisualizationCard->setStyleSheet(
        "QFrame {"
        "background-color: white;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 10px;"
        "}"
    );

    QVBoxLayout *tankVisualizationLayout =
        new QVBoxLayout(tankVisualizationCard);

    QLabel *tankVisualizationTitle =
        new QLabel("Live Tank Level Visualization");

    tankVisualizationTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #1e293b;"
        "border: none;"
        "background-color: transparent;"
    );

    physicsTankVisualizationValue =
    new QLabel("Tank Level: N/A");

    QLabel *tankVisualizationValue =
    physicsTankVisualizationValue;

    tankVisualizationValue->setAlignment(Qt::AlignCenter);

    tankVisualizationValue->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2563eb;"
        "border: none;"
        "background-color: transparent;"
    );

    physicsTankLevelBar = new QProgressBar();
    QProgressBar *tankLevelBar = physicsTankLevelBar;

    tankLevelBar->setOrientation(Qt::Vertical);
    tankLevelBar->setRange(0, 100);
    tankLevelBar->setValue(0);
    tankLevelBar->setTextVisible(true);
    tankLevelBar->setFormat("%p%");
    tankLevelBar->setFixedSize(55, 70);
    tankLevelBar->setAlignment(Qt::AlignCenter);

    tankLevelBar->setStyleSheet(
        "QProgressBar {"
        "background-color: #e2e8f0;"
        "border: 2px solid #94a3b8;"
        "border-radius: 8px;"
        "color: #0f172a;"
        "font-weight: bold;"
        "text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "background-color: #2563eb;"
        "border-radius: 5px;"
        "}"
    );

    tankVisualizationLayout->addWidget(
        tankVisualizationTitle
    );

    tankVisualizationLayout->addWidget(
        tankLevelBar,
        0,
        Qt::AlignHCenter
    );
    // Metrics and tank visualization side by side.
QHBoxLayout *physicsOverviewLayout = new QHBoxLayout();
physicsOverviewLayout->setSpacing(12);
physicsOverviewLayout->setContentsMargins(0, 0, 0, 0);

physicsOverviewLayout->addLayout(physicsGrid, 3);

physicsOverviewLayout->addWidget(
    tankVisualizationCard,
    1,
    Qt::AlignRight | Qt::AlignTop
);

    tankVisualizationLayout->addWidget(
        tankVisualizationValue
    );

// Snapshot Information / Live Data Status

physicsSnapshotInfo = new QLabel(
    "Data Source: Waiting for simulation\n"
    "Data Status: STOPPED"
);

QLabel *snapshotInfo = physicsSnapshotInfo;

physicsSnapshotInfo->setWordWrap(true);
physicsSnapshotInfo->setMinimumHeight(55);
physicsSnapshotInfo->setMaximumHeight(95);
physicsSnapshotInfo->setTextInteractionFlags(
    Qt::TextSelectableByMouse
);

physicsSnapshotInfo->setStyleSheet(
    "QLabel {"
    "background-color: #f8fafc;"
    "color: #334155;"
    "border: 1px solid #cbd5e1;"
    "border-radius: 8px;"
    "padding: 6px 10px;"
    "font-size: 13px;"
    "font-weight: 600;"
    "}"
);

    // Physics Validation Results

    QLabel *validationTitle = new QLabel(
        "Physics Validation Results"
    );

    validationTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    physicsValidationSummary = new QLabel();
    QLabel *validationSummary = physicsValidationSummary;

    physicsViolationLabel = new QLabel();
    QLabel *violationLabel = physicsViolationLabel;

    physicsWarningLabel = new QLabel();
    QLabel *warningLabel = physicsWarningLabel;

    validationSummary->setWordWrap(true);
    violationLabel->setWordWrap(true);
    warningLabel->setWordWrap(true);

validationSummary->setStyleSheet(
    "background-color: #f8fafc;"
    "padding: 4px 8px;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 5px;"
    "color: #334155;"
    "font-size: 12px;"
    "font-weight: bold;"
);

violationLabel->setStyleSheet(
    "background-color: #fffafa;"
    "padding: 4px 8px;"
    "border: 1px solid #fecaca;"
    "border-radius: 5px;"
    "color: #dc2626;"
    "font-size: 12px;"
);

warningLabel->setStyleSheet(
    "background-color: #fffbeb;"
    "padding: 4px 8px;"
    "border: 1px solid #fde68a;"
    "border-radius: 5px;"
    "color: #b45309;"
    "font-size: 12px;"
);

// Live Simulation Validation Results

QLabel *liveValidationTitle = new QLabel(
    "Live Simulation Validation"
);

liveValidationTitle->setStyleSheet(
    "font-size: 17px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
);

livePhysicsValidationSummary = new QLabel(
    "Live Validation: SIMULATION STOPPED"
);

livePhysicsViolationLabel = new QLabel(
    "Live Violations: N/A"
);

livePhysicsWarningLabel = new QLabel(
    "Live Warnings: N/A"
);

livePhysicsValidationSummary->setWordWrap(true);
livePhysicsViolationLabel->setWordWrap(true);
livePhysicsWarningLabel->setWordWrap(true);

livePhysicsValidationSummary->setStyleSheet(
    "background-color: #f8fafc;"
    "padding: 4px 8px;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 5px;"
    "color: #334155;"
    "font-size: 12px;"
    "font-weight: bold;"
);

livePhysicsViolationLabel->setStyleSheet(
    "background-color: #fffafa;"
    "padding: 4px 8px;"
    "border: 1px solid #fecaca;"
    "border-radius: 5px;"
    "color: #dc2626;"
    "font-size: 12px;"
);

livePhysicsWarningLabel->setStyleSheet(
    "background-color: #fffbeb;"
    "padding: 4px 8px;"
    "border: 1px solid #fde68a;"
    "border-radius: 5px;"
    "color: #b45309;"
    "font-size: 12px;"
);

    // Dynamic Scenario Loading

    auto updatePhysicsScenario = [
    this,
        tankValue,
        pumpValue,
        valveValue,
        flowValue,
        tankLevelBar,
        tankVisualizationValue,
        snapshotInfo,
        validationSummary,
        violationLabel,
        warningLabel,
        scenarioSelector
    ]()
    {
        QString selectedFile =
            scenarioSelector->currentData().toString();

        QString snapshotPath = QDir(
            QCoreApplication::applicationDirPath()
        ).absoluteFilePath("../data/generated/" + selectedFile);

        snapshotPath = QDir::cleanPath(snapshotPath);

        // Select the matching combined analysis report

       QString reportFileName;

       if (selectedFile == "process_snapshot.json")
{
    reportFileName = "normal_combined_analysis_report.json";
}
else if (selectedFile == "abnormal_process_snapshot.json")
{
    reportFileName = "abnormal_combined_analysis_report.json";
}
else
{
    reportFileName = "combined_analysis_report.json";
}

QString reportPath = QDir(
    QCoreApplication::applicationDirPath()
).absoluteFilePath("../reports/" + reportFileName);

reportPath = QDir::cleanPath(reportPath);

        tankValue->setText("N/A");
        pumpValue->setText("N/A");
        valveValue->setText("N/A");
        flowValue->setText("N/A");

        snapshotInfo->setText("Data Source: " + selectedFile +
                              "\nEnvironment: LOCAL_TEST");

        snapshotInfo->setText(
    "Data Source: " + selectedFile +
    "\nData Status: LOADING"
);

        validationSummary->setText(
            "Validation Status: NOT AVAILABLE\nDetected Violations: N/A"
        );

        violationLabel->setText("Rule Violations: Not available.");
        warningLabel->setText("Warnings: Not available.");

        // Load selected process snapshot

        QFile snapshotFile(snapshotPath);

        bool snapshotLoaded = false;
        QString snapshotTransaction;

        if (snapshotFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QJsonParseError snapshotError;

            QJsonDocument snapshotDocument = QJsonDocument::fromJson(
                snapshotFile.readAll(),
                &snapshotError
            );

            snapshotFile.close();

            if (snapshotError.error == QJsonParseError::NoError
                && snapshotDocument.isObject())
            {
                QJsonObject root = snapshotDocument.object();

                QJsonObject process =
                    root.value("process_state").toObject();

                if (process.value("tank_level_percent").isDouble()
                    && process.value("pump_on").isBool()
                    && process.value("valve_opening_percent").isDouble()
                    && process.value("flow_rate_lpm").isDouble())
                {
                    double level =
                        process.value("tank_level_percent").toDouble();

                    tankLevelBar->setValue(qRound(level));

                        tankVisualizationValue->setText(
                        "Tank Level: " + QString::number(level, 'f', 1) + "%"
                    );

                    bool pump =
                        process.value("pump_on").toBool();

                    double valve =
                        process.value("valve_opening_percent").toDouble();

                    double flow =
                        process.value("flow_rate_lpm").toDouble();

                    tankValue->setText(
                        QString::number(level, 'f', 1) + "%"
                    );

// Keep the live pump indicator OFF until simulation starts.
// Historical snapshot values must not override the live status.
pumpValue->setText("OFF");

pumpValue->setStyleSheet(
    "color: #dc2626;"
    "font-size: 25px;"
    "font-weight: bold;"
    "background-color: transparent;"
);

                    valveValue->setText(
                        QString::number(valve, 'f', 1) + "%"
                    );

                    flowValue->setText(
                        QString::number(flow, 'f', 1) + " L/min"
                    );

                    snapshotTransaction =
                        QString::number(root.value("transaction_id").toInt());

                    snapshotLoaded = true;
                }
            }
        }

        if (snapshotLoaded)
        {
            snapshotInfo->setText(
                "Data Source: " + selectedFile +
                "\nEnvironment: LOCAL_TEST" +
                "\nTransaction ID: " + snapshotTransaction +
                "\nData Status: SNAPSHOT LOADED"
            );
        }
        else
        {
            snapshotInfo->setText(
                "Data Source: " + selectedFile +
                "\nEnvironment: LOCAL_TEST" +
                "\nData Status: SNAPSHOT COULD NOT BE LOADED"
            );
        }

        // Load matching combined validation report

        QFile reportFile(reportPath);

        if (reportFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QJsonParseError reportError;

            QJsonDocument reportDocument = QJsonDocument::fromJson(
                reportFile.readAll(),
                &reportError
            );

            reportFile.close();

            if (reportError.error == QJsonParseError::NoError
                && reportDocument.isObject())
            {
                QJsonObject reportRoot = reportDocument.object();

                QJsonObject physicsAnalysis =
                    reportRoot.value("physics_analysis").toObject();

                QJsonObject validationResult =
                    physicsAnalysis.value("validation_result").toObject();

                int reportTransaction =
                    physicsAnalysis.value("transaction_id").toInt(-1);

                bool transactionOk = false;
                int selectedTransaction =
                snapshotTransaction.toInt(&transactionOk);

                if (!transactionOk)
                {
                  selectedTransaction = -2;
                }

                // Ensure the report belongs to the selected snapshot

                if (snapshotLoaded
                    && reportTransaction == selectedTransaction
                    && validationResult.value("valid").isBool())
                {
                    bool valid =
                        validationResult.value("valid").toBool();

                    QJsonArray violations =
                        validationResult.value("violations").toArray();

                    QJsonArray warnings =
                        validationResult.value("warnings").toArray();

                    validationSummary->setText(
                        "Validation Status: " +
                        QString(valid ? "VALID" : "ANOMALIES DETECTED") +
                        "\nDetected Violations: " +
                        QString::number(violations.size())
                    );

                    QStringList violationDescriptions;

                    for (const QJsonValue &item : violations)
                    {
                        violationDescriptions.append(item.toString());
                    }

                    violationLabel->setText(
                        "Rule Violations:\n" +
                        (violationDescriptions.isEmpty()
                            ? QString("No physics rule violations detected.")
                            : violationDescriptions.join("\n"))
                    );

                    QStringList warningDescriptions;

                    for (const QJsonValue &item : warnings)
                    {
                        warningDescriptions.append(item.toString());
                    }

                    warningLabel->setText(
                        "Warnings:\n" +
                        (warningDescriptions.isEmpty()
                            ? QString("No physics warnings.")
                            : warningDescriptions.join("\n"))
                    );
                }
                else
                {
                    validationSummary->setText(
                        "Validation Status: REPORT DOES NOT MATCH SELECTED SCENARIO"
                        "\nDetected Violations: N/A"
                    );

                    violationLabel->setText(
                        "Rule Violations: Generate a combined report for this scenario."
                    );

                    warningLabel->setText(
                        "Warnings: Validation results are not available for this scenario."
                    );
                }
            }
            else
            {
                validationSummary->setText(
                    "Validation Status: INVALID REPORT JSON"
                );
            }
        }
        else
        {
            validationSummary->setText(
                "Validation Status: REPORT NOT FOUND"
            );
        }
    };

    QObject::connect(
        scenarioSelector,
        &QComboBox::currentIndexChanged,
        physicsPage,
        [updatePhysicsScenario](int)
        {
            updatePhysicsScenario();
        }
    );

    // Initial scenario

    updatePhysicsScenario();

// Page Layout

QVBoxLayout *physicsHeaderLayout = new QVBoxLayout();
physicsHeaderLayout->setSpacing(0);
physicsHeaderLayout->setContentsMargins(0, 0, 0, 0);

physicsHeaderLayout->addWidget(physicsTitle);
physicsHeaderLayout->addWidget(physicsSubtitle);

physicsLayout->addLayout(physicsHeaderLayout);
physicsLayout->addLayout(scenarioLayout);
physicsLayout->addLayout(physicsOverviewLayout);

// Data Status
physicsLayout->addWidget(snapshotInfo);

// Physics Validation
QVBoxLayout *validationLayout = new QVBoxLayout();
validationLayout->setSpacing(4);
validationLayout->setContentsMargins(0, 0, 0, 0);

validationLayout->addWidget(validationTitle);
validationLayout->addWidget(validationSummary);
validationLayout->addWidget(violationLabel);
validationLayout->addWidget(warningLabel);
validationLayout->addSpacing(12);
validationLayout->addWidget(liveValidationTitle);
validationLayout->addWidget(livePhysicsValidationSummary);
validationLayout->addWidget(livePhysicsViolationLabel);
validationLayout->addWidget(livePhysicsWarningLabel);

physicsLayout->addLayout(validationLayout);
validationLayout->setAlignment(Qt::AlignTop);

stackedWidget->addWidget(physicsPage);

    // =========================================
    // PAGE 5: SECURITY ALERTS
    // =========================================

QWidget *alertsPage = new QWidget(stackedWidget);

QVBoxLayout *alertsLayout = new QVBoxLayout(alertsPage);
alertsLayout->setContentsMargins(20, 18, 20, 18);
alertsLayout->setSpacing(12);

QLabel *alertsTitle = new QLabel("Security Alerts Dashboard");

alertsTitle->setStyleSheet(
    "font-size: 24px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
    "padding: 0px;"
    "margin: 0px;"
);

alertsTitle->setSizePolicy(
    QSizePolicy::Preferred,
    QSizePolicy::Fixed
);
QLabel *alertsSubtitle = new QLabel(
    "Security monitoring and simulated threat detection"
);

alertsSubtitle->setStyleSheet(
    "font-size: 12px;"
    "color: #64748b;"
    "background-color: transparent;"
    "padding: 0px;"
    "margin: 0px;"
);

alertsSubtitle->setSizePolicy(
    QSizePolicy::Preferred,
    QSizePolicy::Fixed
);

    QLabel *scenarioInfo = new QLabel();
    QLabel *protocolSummary = new QLabel();
    QLabel *protocolDetails = new QLabel();
    QLabel *physicsSummary = new QLabel();
    QLabel *decisionSummary = new QLabel();
    liveSecuritySummary = new QLabel(
    "LIVE SECURITY ACTIVITY\n\n"
    "Live Frames: 0\n"
    "Valid Requests: 0\n"
    "Validation Failures: 0\n"
    "Invalid Frames: 0"
);

liveSecuritySummary->setStyleSheet(
    "font-size: 14px;"
    "font-weight: bold;"
    "color: #1e293b;"
    "background-color: #eff6ff;"
    "border: 1px solid #bfdbfe;"
    "border-radius: 8px;"
    "padding: 12px;"
);

liveSecuritySummary->setWordWrap(true);
liveDecisionSummary = new QLabel(
    "LIVE SIMULATED DECISION\n\n"
    "Decision: WAITING\n"
    "Waiting for live protocol and physics evidence."
);

liveDecisionSummary->setStyleSheet(
    "font-size: 14px;"
    "font-weight: bold;"
    "color: #1e293b;"
    "background-color: #f0fdf4;"
    "border: 1px solid #bbf7d0;"
    "border-radius: 8px;"
    "padding: 12px;"
);

liveDecisionSummary->setWordWrap(true);
    QLabel *decisionDetails = new QLabel();
    QLabel *physicsDetails = new QLabel();
    QLabel *interpretationInfo = new QLabel();

    scenarioInfo->setStyleSheet(
    "font-size: 13px;"
    "font-weight: bold;"
    "color: #334155;"
    "background-color: #f1f5f9;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 6px;"
    "padding: 9px 12px;"
);

    protocolSummary->setStyleSheet(
    "font-size: 17px;"
    "font-weight: bold;"
    "color: #1e293b;"
    "background-color: #f8fafc;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 8px;"
    "padding: 10px;"
);

    physicsSummary->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #1e293b;"
    );

    protocolDetails->setStyleSheet(
    "font-size: 13px;"
    "color: #334155;"
    "background-color: #ffffff;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 8px;"
    "padding: 12px;"
);

    physicsDetails->setStyleSheet(
        "font-size: 14px;"
        "color: #334155;"
    );

  decisionSummary->setStyleSheet(
    "font-size: 20px;"
    "font-weight: bold;"
    "color: #b91c1c;"
    "background-color: #f8fafc;"
    "border: 1px solid #cbd5e1;"
    "border-radius: 8px;"
    "padding: 12px;"
);

    decisionDetails->setStyleSheet(
    "font-size: 13px;"
    "color: #334155;"
    "background-color: #ffffff;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 8px;"
    "padding: 12px;"
);

    interpretationInfo->setStyleSheet(
        "font-size: 13px;"
        "color: #475569;"
        "background-color: #f1f5f9;"
        "padding: 10px;"
    );

    scenarioInfo->setWordWrap(true);
    protocolSummary->setWordWrap(true);
    protocolDetails->setWordWrap(true);
    physicsSummary->setWordWrap(true);
    physicsDetails->setWordWrap(true);
    decisionSummary->setWordWrap(true);
    decisionDetails->setWordWrap(true);
    interpretationInfo->setWordWrap(true);

    // Load alerts from the selected scenario report
    auto updateSecurityAlerts = [
        scenarioSelector,
        scenarioInfo,
        protocolSummary,
        protocolDetails,
        physicsSummary,
        decisionSummary,
        decisionDetails,
        physicsDetails,
        interpretationInfo
    ]()
    {
        QString selectedFile =
            scenarioSelector->currentData().toString();

        QString reportFile;

        if (selectedFile == "process_snapshot.json")
        {
            reportFile = "normal_combined_analysis_report.json";
        }
        else if (selectedFile == "abnormal_process_snapshot.json")
        {
            reportFile = "abnormal_combined_analysis_report.json";
        }
        else
        {
            reportFile = "combined_analysis_report.json";
        }

        QString reportPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath("../reports/" + reportFile);

        QFile file(reportPath);

        if (!file.open(QIODevice::ReadOnly))
        {
            scenarioInfo->setText("Selected scenario report");
            protocolSummary->setText("Protocol Analysis: REPORT NOT FOUND");
            protocolDetails->clear();
            physicsSummary->setText("Physics Analysis: REPORT NOT FOUND");
            physicsDetails->clear();
            interpretationInfo->setText(
                "Unable to load the selected combined analysis report."
            );
            decisionSummary->setText("SIMULATED DECISION: NOT AVAILABLE");
            decisionDetails->clear();
            return;
        }

        QByteArray reportData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument document =
            QJsonDocument::fromJson(reportData, &parseError);

        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject())
        {
            scenarioInfo->setText("Selected scenario report");
            protocolSummary->setText("Protocol Analysis: INVALID REPORT");
            protocolDetails->clear();
            physicsSummary->setText("Physics Analysis: INVALID REPORT");
            physicsDetails->clear();
            interpretationInfo->setText(
                "The report could not be parsed as valid JSON."
            );
            decisionSummary->setText("SIMULATED DECISION: NOT AVAILABLE");
            decisionDetails->clear();
            return;
        }

        QJsonObject root = document.object();

                    // Load the simulated Decision Engine report
        QString decisionReportFile;

        if (selectedFile == "process_snapshot.json")
        {
            decisionReportFile = "normal_decision_report.json";
        }
        else if (selectedFile == "abnormal_process_snapshot.json")
        {
            decisionReportFile = "abnormal_decision_report.json";
        }
        else
        {
            decisionReportFile = "combined_decision_report.json";
        }

        QString decisionReportPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath("../reports/" + decisionReportFile);

        QFile decisionFile(decisionReportPath);

        if (!decisionFile.open(QIODevice::ReadOnly))
        {
            decisionSummary->setText(
                "SIMULATED DECISION: REPORT NOT FOUND"
            );
            decisionDetails->setText(
                "Unable to load the Decision Engine report: " +
                decisionReportFile
            );
        }
        else
        {
            QByteArray decisionData = decisionFile.readAll();
            decisionFile.close();

            QJsonParseError decisionParseError;
            QJsonDocument decisionDocument =
                QJsonDocument::fromJson(
                    decisionData,
                    &decisionParseError
                );

            if (decisionParseError.error != QJsonParseError::NoError ||
                !decisionDocument.isObject())
            {
                decisionSummary->setText(
                    "SIMULATED DECISION: INVALID REPORT"
                );
                decisionDetails->setText(
                    "The Decision Engine report contains invalid JSON."
                );
            }
            else
            {
                QJsonObject decisionRoot = decisionDocument.object();
                QJsonObject decisionAnalysis =
                    decisionRoot.value("decision_analysis").toObject();

                QString decision =
                    decisionAnalysis.value("decision").toString("UNKNOWN");

                QJsonObject evidence =
                    decisionAnalysis.value("evidence").toObject();

                decisionSummary->setText(
                    "SIMULATED DECISION: " + decision
                );

                QString decisionColor = "#d97706";

if (decision == "ALLOW")
{
    decisionColor = "#15803d";
}
else if (decision == "BLOCK")
{
    decisionColor = "#dc2626";
}

decisionSummary->setStyleSheet(
    "font-size: 20px;"
    "font-weight: bold;"
    "color: " + decisionColor + ";"
    "background-color: #f8fafc;"
    "border: 1px solid #cbd5e1;"
    "border-radius: 8px;"
    "padding: 12px;"
);

                QString details =
                    "Total Frames: " +
                    QString::number(
                        evidence.value("total_frames").toInt()
                    ) +
                    "\nValid Requests: " +
                    QString::number(
                        evidence.value("valid_requests").toInt()
                    ) +
                    "\nValidation Failures: " +
                    QString::number(
                        evidence.value("validation_failures").toInt()
                    ) +
                    "\nInvalid Frames: " +
                    QString::number(
                        evidence.value("invalid_frames").toInt()
                    ) +
                    "\nMalformed Inputs: " +
                    QString::number(
                        evidence.value("malformed_inputs").toInt()
                    ) +
                    "\nPhysics Valid: " +
                    QString(
                        evidence.value("physics_valid").toBool()
                        ? "YES" : "NO"
                    ) +
                    "\nPhysics Violations: " +
                    QString::number(
                        evidence.value("physics_violations").toInt()
                    ) +
                    "\nPhysics Warnings: " +
                    QString::number(
                        evidence.value("physics_warnings").toInt()
                    );

                QJsonArray reasons =
                    decisionAnalysis.value("reasons").toArray();

                for (const QJsonValue &reason : reasons)
                {
                    details += "\n- " + reason.toString();
                }

                decisionDetails->setText(details);
            }
        }

        // Scenario information
        scenarioInfo->setText(
            "Scenario: " + scenarioSelector->currentText()
        );

        // Protocol analysis
        QJsonObject parserAnalysis =
            root.value("parser_analysis").toObject();

        QJsonObject parserSummary =
            parserAnalysis.value("summary").toObject();

        int totalFrames =
            parserSummary.value("total_frames").toInt();

        int validRequests =
            parserSummary.value("valid_requests").toInt();

        int validationFailures =
            parserSummary.value("validation_failures").toInt();

        int invalidFrames =
            parserSummary.value("invalid_frames").toInt();

        int malformedInputs =
            parserSummary.value("malformed_inputs").toInt();

        protocolSummary->setText(
            "Protocol Analysis"
        );

        protocolDetails->setText(
            "Total Frames: " + QString::number(totalFrames) + "\n"
            "Valid Requests: " + QString::number(validRequests) + "\n"
            "Validation Failures: " + QString::number(validationFailures) + "\n"
            "Invalid Frames: " + QString::number(invalidFrames) + "\n"
            "Malformed Inputs: " + QString::number(malformedInputs)
        );

        // Physics analysis
        QJsonObject physicsAnalysis =
            root.value("physics_analysis").toObject();

        QJsonObject validationResult =
            physicsAnalysis.value("validation_result").toObject();

        bool physicsValid =
            validationResult.value("valid").toBool();

        QJsonArray violations =
            validationResult.value("violations").toArray();

        QJsonArray warnings =
            validationResult.value("warnings").toArray();

        physicsSummary->setText(
        "Physics Analysis: " +
         QString(physicsValid ? "VALID" : "ANOMALY INDICATORS FOUND")
        );

        physicsSummary->setStyleSheet(
    "font-size: 17px;"
    "font-weight: bold;"
    "color: #1e293b;"
    "background-color: #f8fafc;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 8px;"
    "padding: 10px;"
);

        physicsDetails->setStyleSheet(
    "font-size: 13px;"
    "color: #334155;"
    "background-color: #ffffff;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 8px;"
    "padding: 12px;"
);

        if (!violations.isEmpty())
        {
            physicsDetails->setText(
                physicsDetails->text() + "\n\nDetected Violations:"
            );

            for (const QJsonValue &value : violations)
            {
                physicsDetails->setText(
                    physicsDetails->text() + "\n- " + value.toString()
                );
            }
        }

        if (!warnings.isEmpty())
        {
            physicsDetails->setText(
                physicsDetails->text() + "\n\nWarnings:"
            );

            for (const QJsonValue &value : warnings)
            {
                physicsDetails->setText(
                    physicsDetails->text() + "\n- " + value.toString()
                );
            }
        }

        if (violations.isEmpty() && warnings.isEmpty())
        {
            physicsDetails->setText(
                physicsDetails->text() +
                "\nNo physics violations or warnings recorded."
            );
        }

        interpretationInfo->setText(
            "Interpretation: Protocol validation failures and physics "
            "anomalies are indicators requiring investigation. "
            "They do not independently establish malicious intent. "
            "All displayed results come from local synthetic simulations."
        );
    };

    // Refresh alerts when the scenario changes
    QObject::connect(
        scenarioSelector,
        &QComboBox::currentIndexChanged,
        alertsPage,
        [updateSecurityAlerts](int)
        {
            updateSecurityAlerts();
        }
    );

    // Load the initial scenario
    updateSecurityAlerts();

    alertsLayout->addWidget(alertsTitle);
alertsLayout->addWidget(alertsSubtitle);
alertsLayout->addWidget(scenarioInfo);

// Side-by-side live security and decision panels.
QHBoxLayout *livePanelsLayout = new QHBoxLayout();
livePanelsLayout->setSpacing(12);

liveSecuritySummary->setMinimumHeight(175);
liveDecisionSummary->setMinimumHeight(175);

liveSecuritySummary->setSizePolicy(
    QSizePolicy::Expanding,
    QSizePolicy::Preferred
);

liveDecisionSummary->setSizePolicy(
    QSizePolicy::Expanding,
    QSizePolicy::Preferred
);

liveSecuritySummary->setAlignment(
    Qt::AlignTop | Qt::AlignLeft
);

liveDecisionSummary->setAlignment(
    Qt::AlignTop | Qt::AlignLeft
);

livePanelsLayout->addWidget(liveSecuritySummary, 1);
livePanelsLayout->addWidget(liveDecisionSummary, 1);

alertsLayout->addSpacing(6);
alertsLayout->addLayout(livePanelsLayout);

// Live Security Event History
QLabel *liveHistoryTitle = new QLabel("LIVE SECURITY EVENT HISTORY");

liveHistoryTitle->setStyleSheet(
    "font-size: 16px;"
    "font-weight: bold;"
    "color: #0f172a;"
    "background-color: transparent;"
    "padding: 6px 0px;"
);

liveSecurityEventTable = new QTableWidget();

liveSecurityEventTable->setColumnCount(4);
liveSecurityEventTable->setHorizontalHeaderLabels({
    "Frame",
    "Status",
    "Decision",
    "Reason"
});

liveSecurityEventTable->setEditTriggers(
    QAbstractItemView::NoEditTriggers
);

liveSecurityEventTable->setSelectionBehavior(
    QAbstractItemView::SelectRows
);

liveSecurityEventTable->setSelectionMode(
    QAbstractItemView::SingleSelection
);

liveSecurityEventTable->verticalHeader()->setVisible(false);
liveSecurityEventTable->horizontalHeader()->setStretchLastSection(true);
liveSecurityEventTable->setAlternatingRowColors(true);
liveSecurityEventTable->setMaximumHeight(260);
liveSecurityEventTable->setMinimumHeight(150);
liveSecurityEventTable->horizontalHeader()->setVisible(true);

liveSecurityEventTable->horizontalHeader()->setMinimumHeight(32);

liveSecurityEventTable->setStyleSheet(
    "QTableWidget {"
    "background-color: #ffffff;"
    "color: #1e293b;"
    "gridline-color: #cbd5e1;"
    "border: 1px solid #cbd5e1;"
    "font-size: 13px;"
    "}"
    "QTableWidget::item {"
    "color: #1e293b;"
    "padding: 6px;"
    "}"
    "QHeaderView::section {"
    "background-color: #e2e8f0;"
    "color: #0f172a;"
    "font-weight: bold;"
    "font-size: 13px;"
    "padding: 8px;"
    "border: 1px solid #cbd5e1;"
    "}"
);

alertsLayout->addSpacing(6);
alertsLayout->addWidget(liveHistoryTitle);
alertsLayout->addWidget(liveSecurityEventTable);

// Three-column historical security analysis layout.
QGridLayout *historicalAnalysisLayout = new QGridLayout();
historicalAnalysisLayout->setSpacing(12);
historicalAnalysisLayout->setContentsMargins(0, 0, 0, 0);

decisionSummary->setMinimumHeight(75);
protocolSummary->setMinimumHeight(75);
physicsSummary->setMinimumHeight(75);

decisionDetails->setMinimumHeight(150);
protocolDetails->setMinimumHeight(150);
physicsDetails->setMinimumHeight(150);

decisionDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);
protocolDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);
physicsDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);

historicalAnalysisLayout->addWidget(decisionSummary, 0, 0);
historicalAnalysisLayout->addWidget(protocolSummary, 0, 1);
historicalAnalysisLayout->addWidget(physicsSummary, 0, 2);

historicalAnalysisLayout->addWidget(decisionDetails, 1, 0);
historicalAnalysisLayout->addWidget(protocolDetails, 1, 1);
historicalAnalysisLayout->addWidget(physicsDetails, 1, 2);

historicalAnalysisLayout->setColumnStretch(0, 1);
historicalAnalysisLayout->setColumnStretch(1, 1);
historicalAnalysisLayout->setColumnStretch(2, 1);

alertsLayout->addSpacing(6);
alertsLayout->addLayout(historicalAnalysisLayout);

alertsLayout->addSpacing(6);
alertsLayout->addWidget(interpretationInfo);

alertsLayout->addStretch();

    stackedWidget->addWidget(alertsPage);


    // =========================================
    // PAGE 6: REPORTS
    // =========================================

    QWidget *reportsPage = new QWidget(stackedWidget);
    QVBoxLayout *reportsLayout = new QVBoxLayout(reportsPage);

    reportsLayout->setContentsMargins(24, 20, 24, 20);
    reportsLayout->setSpacing(12);

    // Header

    QLabel *reportsTitle = new QLabel("Reports & Evidence Center");

    reportsTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background: transparent;"
    );

    QLabel *reportsSubtitle = new QLabel(
        "Review generated analysis results, protocol reports and security evidence."
    );

    reportsSubtitle->setWordWrap(true);
    reportsSubtitle->setStyleSheet(
        "font-size: 14px;"
        "color: #64748b;"
        "background: transparent;"
    );

    reportsLayout->addWidget(reportsTitle);
    reportsLayout->addWidget(reportsSubtitle);
    reportsLayout->addSpacing(8);

    // Report paths

    QString reportsPath = QDir(
        QCoreApplication::applicationDirPath()
    ).absoluteFilePath("../reports");

    QString dataPath = QDir(
        QCoreApplication::applicationDirPath()
    ).absoluteFilePath("../data/generated");

    // Report availability

    bool combinedAvailable =
        QFile::exists(QDir(reportsPath).filePath(
            "normal_combined_analysis_report.json"
        )) &&
        QFile::exists(QDir(reportsPath).filePath(
            "abnormal_combined_analysis_report.json"
        ));

    bool protocolAvailable =
        QFile::exists(QDir(dataPath).filePath(
            "parser_results.json"
        ));

    bool decisionAvailable =
        QFile::exists(QDir(reportsPath).filePath(
            "normal_decision_report.json"
        )) &&
        QFile::exists(QDir(reportsPath).filePath(
            "abnormal_decision_report.json"
        ));

    // Report summary cards

    QHBoxLayout *reportCardsLayout = new QHBoxLayout();
    reportCardsLayout->setSpacing(12);

    QString reportCardStyle =
        "QFrame {"
        "background-color: #1e293b;"
        "border-radius: 8px;"
        "}"
        "QLabel {"
        "color: #f8fafc;"
        "background: transparent;"
        "}";

    auto createReportCard = [&](const QString &heading,
                                const QString &description,
                                bool available)
    {
        QFrame *card = new QFrame();
        card->setStyleSheet(reportCardStyle);
        card->setMinimumHeight(105);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 12, 14, 12);
        cardLayout->setSpacing(5);

        QLabel *headingLabel = new QLabel(heading);
        headingLabel->setStyleSheet(
            "font-size: 15px;"
            "font-weight: bold;"
        );

        QLabel *descriptionLabel = new QLabel(description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet(
            "font-size: 12px;"
            "color: #cbd5e1;"
        );

        QLabel *statusLabel = new QLabel(
            available ? "AVAILABLE" : "MISSING"
        );

        statusLabel->setStyleSheet(
            QString(
                "font-size: 12px;"
                "font-weight: bold;"
                "color: %1;"
            ).arg(available ? "#86efac" : "#fca5a5")
        );

        cardLayout->addWidget(headingLabel);
        cardLayout->addWidget(descriptionLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(statusLabel);

        return card;
    };

    reportCardsLayout->addWidget(
        createReportCard(
            "COMBINED ANALYSIS",
            "Protocol and Physics reports",
            combinedAvailable
        )
    );

    reportCardsLayout->addWidget(
        createReportCard(
            "PROTOCOL REPORT",
            "Modbus/TCP parser results",
            protocolAvailable
        )
    );

    reportCardsLayout->addWidget(
        createReportCard(
            "DECISION REPORT",
            "Security assessment results",
            decisionAvailable
        )
    );

    reportsLayout->addLayout(reportCardsLayout);

    // Analysis control panel

    QFrame *analysisPanel = new QFrame();

    analysisPanel->setStyleSheet(
        "QFrame {"
        "background-color: #f8fafc;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "}"
    );

    QHBoxLayout *analysisPanelLayout =
        new QHBoxLayout(analysisPanel);

    analysisPanelLayout->setContentsMargins(16, 12, 16, 12);

    QVBoxLayout *analysisTextLayout = new QVBoxLayout();

    QLabel *analysisTitle = new QLabel("Generate Analysis Reports");
    analysisTitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background: transparent;"
    );

    QLabel *analysisDescription = new QLabel(
        "Run the analysis using the selected historical scenario."
    );

    analysisDescription->setWordWrap(true);
    analysisDescription->setStyleSheet(
        "font-size: 12px;"
        "color: #64748b;"
        "background: transparent;"
    );

    analysisTextLayout->addWidget(analysisTitle);
    analysisTextLayout->addWidget(analysisDescription);

    QPushButton *runAnalysisButton =
        new QPushButton("Run Analysis");

    runAnalysisButton->setMinimumHeight(42);
    runAnalysisButton->setMinimumWidth(160);
    runAnalysisButton->setStyleSheet(
        "QPushButton {"
        "color: white;"
        "background-color: #2563eb;"
        "border: none;"
        "border-radius: 6px;"
        "padding: 10px 16px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #1d4ed8;"
        "}"
        "QPushButton:disabled {"
        "background-color: #94a3b8;"
        "}"
    );

    analysisPanelLayout->addLayout(analysisTextLayout, 1);
    analysisPanelLayout->addWidget(runAnalysisButton);

    reportsLayout->addWidget(analysisPanel);

    QLabel *analysisStatusLabel =
        new QLabel("Analysis Status: READY");

    analysisStatusLabel->setWordWrap(true);
    analysisStatusLabel->setStyleSheet(
        "color: #15803d;"
        "font-size: 13px;"
        "font-weight: bold;"
        "background-color: #f0fdf4;"
        "padding: 10px;"
        "border-radius: 6px;"
    );

    reportsLayout->addWidget(analysisStatusLabel);

    // Generated reports

    QLabel *generatedReportsTitle =
        new QLabel("Generated Reports");

    generatedReportsTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background: transparent;"
    );

    reportsLayout->addWidget(generatedReportsTitle);

    QGridLayout *reportButtonsLayout = new QGridLayout();
    reportButtonsLayout->setHorizontalSpacing(12);
    reportButtonsLayout->setVerticalSpacing(10);

    auto createReportButton = [](const QString &text,
                                 const QString &background)
    {
        QPushButton *button = new QPushButton(text);
        button->setMinimumHeight(42);
        button->setStyleSheet(
            QString(
                "QPushButton {"
                "background-color: %1;"
                "color: white;"
                "font-size: 13px;"
                "font-weight: bold;"
                "padding: 9px;"
                "border: none;"
                "border-radius: 6px;"
                "}"
                "QPushButton:hover {"
                "background-color: #334155;"
                "}"
            ).arg(background)
        );

        return button;
    };

    QPushButton *openCombinedReportButton =
        createReportButton(
            "Open Normal Combined Report", "#0f766e"
        );

    QPushButton *openAbnormalCombinedReportButton =
        createReportButton(
            "Open Abnormal Combined Report", "#be123c"
        );

    QPushButton *openProtocolReportButton =
        createReportButton(
            "Open Protocol Parser Report", "#7c3aed"
        );

    QPushButton *openDecisionReportButton =
        createReportButton(
            "Open Normal Security Decision Report", "#b45309"
        );

    QPushButton *openAbnormalDecisionReportButton =
        createReportButton(
            "Open Abnormal Security Decision Report", "#991b1b"
        );

    reportButtonsLayout->addWidget(
        openCombinedReportButton, 0, 0
    );

    reportButtonsLayout->addWidget(
        openAbnormalCombinedReportButton, 0, 1
    );

    reportButtonsLayout->addWidget(
        openProtocolReportButton, 1, 0
    );

    reportButtonsLayout->addWidget(
        openDecisionReportButton, 1, 1
    );

    reportButtonsLayout->addWidget(
        openAbnormalDecisionReportButton, 2, 0, 1, 2
    );

    reportsLayout->addLayout(reportButtonsLayout);

    // Report storage

    QFrame *storagePanel = new QFrame();

    storagePanel->setStyleSheet(
        "QFrame {"
        "background-color: #f1f5f9;"
        "border-radius: 8px;"
        "}"
    );

    QHBoxLayout *storageLayout = new QHBoxLayout(storagePanel);
    storageLayout->setContentsMargins(14, 10, 14, 10);

    QLabel *reportLocationInfo = new QLabel(
        "Storage: ../reports/\n"
        "Protocol data: ../data/generated/"
    );

    reportLocationInfo->setWordWrap(true);
    reportLocationInfo->setStyleSheet(
        "font-size: 12px;"
        "color: #475569;"
        "background: transparent;"
    );

    QPushButton *openReportsFolderButton =
        createReportButton("Open Reports Folder", "#2563eb");

    storageLayout->addWidget(reportLocationInfo, 1);
    storageLayout->addWidget(openReportsFolderButton);

    reportsLayout->addWidget(storagePanel);
    reportsLayout->addStretch();

    // Report opening connections

    QObject::connect(
        openCombinedReportButton,
        &QPushButton::clicked,
        reportsPage,
        [reportsPath]()
        {
            QString reportFile = QDir(reportsPath).filePath(
                "normal_combined_analysis_report.json"
            );

            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportFile)
            );
        }
    );

    QObject::connect(
        openAbnormalCombinedReportButton,
        &QPushButton::clicked,
        reportsPage,
        [reportsPath]()
        {
            QString reportFile = QDir(reportsPath).filePath(
                "abnormal_combined_analysis_report.json"
            );

            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportFile)
            );
        }
    );

    QObject::connect(
        openProtocolReportButton,
        &QPushButton::clicked,
        reportsPage,
        [dataPath]()
        {
            QString reportFile = QDir(dataPath).filePath(
                "parser_results.json"
            );

            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportFile)
            );
        }
    );

    QObject::connect(
        openDecisionReportButton,
        &QPushButton::clicked,
        reportsPage,
        [reportsPath]()
        {
            QString reportFile = QDir(reportsPath).filePath(
                "normal_decision_report.json"
            );

            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportFile)
            );
        }
    );

    QObject::connect(
        openAbnormalDecisionReportButton,
        &QPushButton::clicked,
        reportsPage,
        [reportsPath]()
        {
            QString reportFile = QDir(reportsPath).filePath(
                "abnormal_decision_report.json"
            );

            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportFile)
            );
        }
    );

    QObject::connect(
        openReportsFolderButton,
        &QPushButton::clicked,
        reportsPage,
        [reportsPath]()
        {
            QDesktopServices::openUrl(
                QUrl::fromLocalFile(reportsPath)
            );
        }
    );

    // Run Python analysis when requested

    QObject::connect(
        runAnalysisButton,
        &QPushButton::clicked,
        reportsPage,
        [this,
         runAnalysisButton,
         analysisStatusLabel,
         scenarioSelector,
         updatePhysicsScenario,
         updateSecurityAlerts]()
        {
            if (analysisProcess->state() != QProcess::NotRunning)
            {
                return;
            }

            QString packageRoot = QDir(
                QCoreApplication::applicationDirPath()
            ).absoluteFilePath("..");

            packageRoot = QDir::cleanPath(packageRoot);

            QString pythonExecutable = QDir(packageRoot)
                .filePath(".venv/Scripts/python.exe");

            QString scriptPath = QDir(packageRoot)
                .filePath("run_analysis.py");

            if (!QFile::exists(pythonExecutable))
            {
                analysisStatusLabel->setText(
                    "Analysis Status: Python environment not found.\n"
                    "Install dependencies using INSTALLATION.md."
                );

                analysisStatusLabel->setStyleSheet(
                    "color: #b45309;"
                    "font-size: 14px;"
                    "font-weight: bold;"
                    "background-color: #fffbeb;"
                    "padding: 10px;"
                    "border-radius: 6px;"
                );

                return;
            }

            if (!QFile::exists(scriptPath))
            {
                analysisStatusLabel->setText(
                    "Analysis Status: run_analysis.py not found."
                );

                analysisStatusLabel->setStyleSheet(
                    "color: #dc2626;"
                    "font-size: 14px;"
                    "font-weight: bold;"
                    "background-color: #fef2f2;"
                    "padding: 10px;"
                    "border-radius: 6px;"
                );

                return;
            }

            QString scenario =
                scenarioSelector->currentIndex() == 0
                ? "normal"
                : "abnormal";

            runAnalysisButton->setEnabled(false);

            analysisStatusLabel->setText(
                "Analysis Status: RUNNING - " +
                scenario.toUpper() +
                " scenario..."
            );

            analysisProcess->setWorkingDirectory(packageRoot);

            analysisProcess->start(
                pythonExecutable,
                QStringList() << scriptPath << scenario
            );
        }
    );

    // Handle successful or unsuccessful process completion

    QObject::connect(
        analysisProcess,
        &QProcess::finished,
        reportsPage,
        [runAnalysisButton,
         analysisStatusLabel,
         updatePhysicsScenario,
         updateSecurityAlerts]
        (int exitCode, QProcess::ExitStatus exitStatus)
        {
            runAnalysisButton->setEnabled(true);

            if (exitStatus == QProcess::NormalExit && exitCode == 0)
            {
                analysisStatusLabel->setText(
                    "Analysis Status: COMPLETED SUCCESSFULLY.\n"
                    "Reports have been generated."
                );

                analysisStatusLabel->setStyleSheet(
                    "color: #15803d;"
                    "font-size: 14px;"
                    "font-weight: bold;"
                    "background-color: #f0fdf4;"
                    "padding: 10px;"
                    "border-radius: 6px;"
                );

                updatePhysicsScenario();
                updateSecurityAlerts();
            }
            else
            {
                analysisStatusLabel->setText(
                    "Analysis Status: FAILED.\n"
                    "Exit code: " + QString::number(exitCode)
                );

                analysisStatusLabel->setStyleSheet(
                    "color: #dc2626;"
                    "font-size: 14px;"
                    "font-weight: bold;"
                    "background-color: #fef2f2;"
                    "padding: 10px;"
                    "border-radius: 6px;"
                );
            }
        }
    );

    // Handle process startup and runtime errors

    QObject::connect(
        analysisProcess,
        &QProcess::errorOccurred,
        reportsPage,
        [this, runAnalysisButton, analysisStatusLabel]
        (QProcess::ProcessError error)
        {
            Q_UNUSED(error);

            runAnalysisButton->setEnabled(true);

            analysisStatusLabel->setText(
                "Analysis Status: PROCESS ERROR.\n" +
                analysisProcess->errorString()
            );

            analysisStatusLabel->setStyleSheet(
                "color: #dc2626;"
                "font-size: 14px;"
                "font-weight: bold;"
                "background-color: #fef2f2;"
                "padding: 10px;"
                "border-radius: 6px;"
            );
        }
    );

    stackedWidget->addWidget(reportsPage);

}
// =========================================
// DASHBOARD PAGE NAVIGATION
// =========================================

void MainWindow::switchPage(int pageIndex)
{
    if (stackedWidget == nullptr)
        return;

    if (pageIndex < 0 || pageIndex >= stackedWidget->count())
        return;

    stackedWidget->setCurrentIndex(pageIndex);

    QList<QPushButton *> buttons = {
        overviewButton,
        networkButton,
        protocolButton,
        physicsButton,
        alertsButton,
        reportsButton
    };

    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttons[i] == nullptr)
            continue;

        buttons[i]->setStyleSheet(
            i == pageIndex
                ? "QPushButton {"
                  "background-color: #2563eb;"
                  "color: white;"
                  "border: none;"
                  "border-radius: 8px;"
                  "padding: 12px;"
                  "text-align: left;"
                  "font-weight: bold;"
                  "}"
                : "QPushButton {"
                  "background-color: transparent;"
                  "color: #ffffff;"
                  "border: none;"
                  "border-radius: 8px;"
                  "padding: 12px;"
                  "text-align: left;"
                  "font-weight: 500;"
                  "}"
                  "QPushButton:hover {"
                  "background-color: #e2e8f0;"
                  "color: #0f172a;"
                  "}"
        );
    }
}

// Handle continuous JSON Lines from the Python simulation
void MainWindow::handleLiveDecisionOutput()
{
    if (liveDecisionProcess == nullptr)
        return;

    liveDecisionOutputBuffer.append(
        liveDecisionProcess->readAllStandardOutput()
    );

    while (liveDecisionOutputBuffer.contains('\n'))
    {
        int newlineIndex =
            liveDecisionOutputBuffer.indexOf('\n');

        QByteArray jsonLine =
            liveDecisionOutputBuffer.left(newlineIndex).trimmed();

        liveDecisionOutputBuffer.remove(0, newlineIndex + 1);

        if (jsonLine.isEmpty())
            continue;

        QJsonParseError parseError;

        QJsonDocument document =
            QJsonDocument::fromJson(jsonLine, &parseError);

        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject())
        {
            liveDecisionSummary->setText(
                "LIVE SIMULATED DECISION: INVALID BRIDGE RESPONSE"
            );
            continue;
        }

        QJsonObject root = document.object();

        if (root.value("data_status").toString() == "ERROR")
        {
            liveDecisionSummary->setText(
                "LIVE SIMULATED DECISION: BRIDGE ERROR\n\n" +
                root.value("error").toString()
            );
            continue;
        }

        QJsonObject decision =
            root.value("decision_analysis").toObject();

        QString decisionValue =
            decision.value("decision").toString("UNKNOWN");

        QString interpretation =
            decision.value("interpretation").toString();

        QJsonObject evidence =
            decision.value("evidence").toObject();

        QString details =
            "LIVE SIMULATED DECISION\n\n"
            "Decision: " + decisionValue +
            "\nFrame: " +
            QString::number(root.value("frame_number").toInt()) +
            "\nFrame Status: " +
            root.value("frame_status").toString() +
            "\nTotal Frames: " +
            QString::number(evidence.value("total_frames").toInt()) +
            "\nValid Requests: " +
            QString::number(evidence.value("valid_requests").toInt()) +
            "\nValidation Failures: " +
            QString::number(evidence.value("validation_failures").toInt()) +
            "\nInvalid Frames: " +
            QString::number(evidence.value("invalid_frames").toInt()) +
            "\n\nInterpretation:\n" + interpretation;

        liveDecisionSummary->setText(details);

        // Add the latest event to Live Security Event History.
if (liveSecurityEventTable != nullptr)
{
    int row = 0;

    liveSecurityEventTable->insertRow(row);

    int frameNumber = root.value("frame_number").toInt();
    QString frameStatus = root.value("frame_status").toString();
    QString frameReason = root.value("frame_reason").toString();

    liveSecurityEventTable->setItem(
        row, 0,
        new QTableWidgetItem(QString::number(frameNumber))
    );

    liveSecurityEventTable->setItem(
        row, 1,
        new QTableWidgetItem(frameStatus)
    );

    liveSecurityEventTable->setItem(
        row, 2,
        new QTableWidgetItem(decisionValue)
    );

    liveSecurityEventTable->setItem(
        row, 3,
        new QTableWidgetItem(
            frameReason.isEmpty() ? "No reason provided" : frameReason
        )
    );

    // Keep only the latest 100 events.
    while (liveSecurityEventTable->rowCount() > 100)
    {
        liveSecurityEventTable->removeRow(
            liveSecurityEventTable->rowCount() - 1
        );
    }
}

        if (decisionValue == "ALLOW")
        {
            liveDecisionSummary->setStyleSheet(
                "font-size: 14px;"
                "font-weight: bold;"
                "color: #166534;"
                "background-color: #f0fdf4;"
                "border: 1px solid #bbf7d0;"
                "border-radius: 8px;"
                "padding: 12px;"
            );
        }
        else if (decisionValue == "HOLD")
        {
            liveDecisionSummary->setStyleSheet(
                "font-size: 14px;"
                "font-weight: bold;"
                "color: #92400e;"
                "background-color: #fffbeb;"
                "border: 1px solid #fde68a;"
                "border-radius: 8px;"
                "padding: 12px;"
            );
        }
        else if (decisionValue == "DROP")
        {
            liveDecisionSummary->setStyleSheet(
                "font-size: 14px;"
                "font-weight: bold;"
                "color: #991b1b;"
                "background-color: #fef2f2;"
                "border: 1px solid #fecaca;"
                "border-radius: 8px;"
                "padding: 12px;"
            );
        }
    }
}

// Handle continuous JSON Lines from the Python simulation
void MainWindow::handleLiveOutput()
{
    if (liveSimulationProcess == nullptr)
        return;

    // Append newly received output to the buffer.
    liveOutputBuffer.append(
        liveSimulationProcess->readAllStandardOutput()
    );

    // Extract every complete JSON line.
    int newlineIndex = -1;

    while ((newlineIndex = liveOutputBuffer.indexOf('\n')) >= 0)
    {
        QByteArray jsonLine =
            liveOutputBuffer.left(newlineIndex).trimmed();

        liveOutputBuffer.remove(0, newlineIndex + 1);

        if (!jsonLine.isEmpty())
        {
            processLiveSample(jsonLine);
        }
    }

    // Prevent an unexpectedly large incomplete message.
    if (liveOutputBuffer.size() > 1024 * 1024)
    {
        liveOutputBuffer.clear();
        // Update Physics Monitoring Data Status.
        if (physicsSnapshotInfo != nullptr)
{
        physicsSnapshotInfo->setText(
        "Data Source: VoltGuard Live Simulation Engine\n"
        "Data Status: STOPPED"
    );
}

        liveConnectionLabel->setText(
            "Simulation Error: Invalid output buffer"
        );
    }
}
// Parse one live simulation sample and update the dashboard
void MainWindow::processLiveSample(const QByteArray &jsonLine)
{
        // Ignore delayed samples after the simulation has stopped.
    if (liveSimulationProcess == nullptr ||
        liveSimulationProcess->state() == QProcess::NotRunning)
    {
        return;
    }
    QJsonParseError parseError;

    QJsonDocument document =
        QJsonDocument::fromJson(jsonLine, &parseError);

    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject())
    {
        liveConnectionLabel->setText(
            "Simulation Error: Invalid JSON"
        );
        return;
    }

    QJsonObject root = document.object();

// Accept only live simulation data.
if (root.value("data_source").toString() != "SIMULATION" ||
    root.value("data_status").toString() != "LIVE")
{
    return;
}
    QJsonObject processState =
        root.value("process_state").toObject();

    QJsonObject validation =
        root.value("validation").toObject();

    if (!root.contains("process_state") ||
        !root.value("process_state").isObject() ||
        !root.contains("validation") ||
        !root.value("validation").isObject())
    {
        liveConnectionLabel->setText(
            "Simulation Error: Incomplete sample"
        );
        return;
    }

    // Forward accepted live samples to the independent decision bridge.
if (liveDecisionProcess != nullptr &&
    liveDecisionProcess->state() == QProcess::Running &&
    root.value("modbus").isObject())
{
    QJsonObject modbusEvent =
        root.value("modbus").toObject();

    if (modbusEvent.value("data_source").toString() == "SIMULATION" &&
        modbusEvent.value("data_status").toString() == "LIVE")
    {
        QByteArray decisionInput = jsonLine;
        decisionInput.append('\n');
        liveDecisionProcess->write(decisionInput);
    }
}

    // Read actual simulation values.
    double volume =
        processState.value("tank_volume_liters").toDouble(-1.0);

    double level =
        processState.value("tank_level_percent").toDouble(-1.0);

    double capacity =
        processState.value("tank_capacity_liters").toDouble(-1.0);

    double inletFlow =
        processState.value("inflow_lpm").toDouble(-1.0);

    double outletFlow =
        processState.value("outflow_lpm").toDouble(-1.0);

    bool pumpOn =
        processState.value("pump_on").toBool();

    if (volume < 0.0 || capacity <= 0.0 ||
        level < 0.0 || level > 100.0 ||
        inletFlow < 0.0 || outletFlow < 0.0)
    {
        liveConnectionLabel->setText(
            "Simulation Error: Invalid process values"
        );
        return;
    }

    // Update tank volume and level.
    tankVolumeValueLabel->setText(
        QString("%1 / %2 L")
            .arg(volume, 0, 'f', 2)
            .arg(capacity, 0, 'f', 2)
    );

    tankLevelValueLabel->setText(
        QString("%1%").arg(level, 0, 'f', 2)
    );

    tankLevelProgress->setValue(qBound(0, qRound(level), 100));

    tankLevelProgress->setFormat(
        QString("%1%").arg(level, 0, 'f', 2)
    );

    // Update pump state and colour.
pumpStatusValueLabel->setText(
    pumpOn ? "ON" : "OFF"
);

pumpStatusValueLabel->setStyleSheet(
    pumpOn
        ? "color: #16a34a; font-size: 21px; font-weight: bold; border: none; background: transparent;"
        : "color: #dc2626; font-size: 21px; font-weight: bold; border: none; background: transparent;"
);

    // Update actual inlet and outlet flow.
    inletFlowValueLabel->setText(
        QString("%1 L/min").arg(inletFlow, 0, 'f', 2)
    );

    outletFlowValueLabel->setText(
        QString("%1 L/min").arg(outletFlow, 0, 'f', 2)
    );

    // Update physics validation.
    bool valid = validation.value("valid").toBool(false);

    QJsonArray violations =
        validation.value("violations").toArray();

    QJsonArray warnings =
        validation.value("warnings").toArray();

    QString validationText;

    if (!valid || !violations.isEmpty())
    {
        validationText = "INVALID";
    }
    else if (!warnings.isEmpty())
    {
        validationText = "WARNING";
    }
    else
    {
        validationText = "VALID";
    }

    // Update physics validation status and colour.
validationValueLabel->setText(validationText);

validationValueLabel->setStyleSheet(
    validationText == "VALID"
        ? "color: #16a34a; font-size: 21px; font-weight: bold; border: none; background: transparent;"
        : "color: #dc2626; font-size: 21px; font-weight: bold; border: none; background: transparent;"
);

    // Add the latest level to the live chart.
    QString scenario =
    root.value("scenario").toString();

    tankLevelChart->addDataPoint(scenario, level);
    // =========================================
    // UPDATE PHYSICS MONITORING - LIVE DATA
    // =========================================

double valveOpening =
    processState.value("valve_opening_percent").toDouble(0.0);

double flowRate =
    processState.value("flow_rate_lpm").toDouble(inletFlow);

// Update Physics Monitoring metric cards.
if (physicsTankValue != nullptr)
{
    physicsTankValue->setText(
        QString::number(level, 'f', 1) + "%"
    );
}

if (physicsPumpValue != nullptr)
{
    physicsPumpValue->setText(pumpOn ? "ON" : "OFF");

    physicsPumpValue->setStyleSheet(
        QString(
            "color: %1; font-size: 25px; font-weight: bold;"
        ).arg(pumpOn ? "#16a34a" : "#dc2626")
    );
}

if (physicsValveValue != nullptr)
{
    physicsValveValue->setText(
        QString::number(valveOpening, 'f', 1) + "%"
    );
}

if (physicsFlowValue != nullptr)
{
    physicsFlowValue->setText(
        QString::number(flowRate, 'f', 1) + " L/min"
    );
}

// Update tank visualization.
if (physicsTankLevelBar != nullptr)
{
    physicsTankLevelBar->setValue(
        qBound(0, qRound(level), 100)
    );
}

if (physicsTankVisualizationValue != nullptr)
{
    physicsTankVisualizationValue->setText(
        "Tank Level: " + QString::number(level, 'f', 1) + "%"
    );
}

// Update Physics Monitoring data status.
if (physicsSnapshotInfo != nullptr)
{
    physicsSnapshotInfo->setText(
        QString(
            "Data Source: VoltGuard Live Simulation Engine\n"
            "Data Status: LIVE\n"
            "Scenario: %1\n"
            "Tank Volume: %2 / %3 L"
        )
        .arg(root.value("scenario_description").toString())
        .arg(volume, 0, 'f', 2)
        .arg(capacity, 0, 'f', 2)
    );
}

// Update LIVE Physics Monitoring validation.
if (livePhysicsValidationSummary != nullptr)
{
    livePhysicsValidationSummary->setText(
        "Live Physics Validation: " + validationText
    );
}

QStringList violationMessages;

for (const QJsonValue &item : violations)
{
    violationMessages.append(item.toString());
}

QStringList warningMessages;

for (const QJsonValue &item : warnings)
{
    warningMessages.append(item.toString());
}

if (livePhysicsViolationLabel != nullptr)
{
    livePhysicsViolationLabel->setText(
        violationMessages.isEmpty()
            ? "Live Violations: None"
            : "Live Violations:\n- " + violationMessages.join("\n- ")
    );
}

if (livePhysicsWarningLabel != nullptr)
{
    livePhysicsWarningLabel->setText(
        warningMessages.isEmpty()
            ? "Live Warnings: None"
            : "Live Warnings:\n- " + warningMessages.join("\n- ")
    );
}

    // Read the synthetic Modbus event.
    QJsonObject modbusEvent =
        root.value("modbus").toObject();

    // Process live Modbus data independently of the Network Traffic table.
    if (root.value("modbus").isObject() &&
        modbusEvent.value("data_source").toString() == "SIMULATION" &&
        modbusEvent.value("data_status").toString() == "LIVE")
    {
        // Update live Modbus traffic counters.
        ++liveTotalFrames;

        QString liveStatus =
            modbusEvent.value("status").toString();

        if (liveStatus == "VALID")
        {
            ++liveValidCount;
        }
        else if (liveStatus == "VALIDATION_FAILED")
        {
            ++liveValidationFailureCount;
        }
        else if (liveStatus == "INVALID_FRAME")
        {
            ++liveInvalidFrameCount;
        }

        // Update scenario-specific traffic history.
QString scenario = root.value("scenario").toString();

if (scenario == "normal_filling")
{
    if (liveStatus == "VALID")
        ++normalLiveValidCount;
    else if (liveStatus == "VALIDATION_FAILED")
        ++normalLiveValidationFailureCount;
    else if (liveStatus == "INVALID_FRAME")
        ++normalLiveInvalidCount;

    if (trafficChart != nullptr)
    {
        trafficChart->addDataPoint(
            scenario,
            normalLiveValidCount,
            normalLiveValidationFailureCount,
            normalLiveInvalidCount
        );
    }
}
else if (scenario == "pump_off_consumption")
{
    if (liveStatus == "VALID")
        ++pumpOffLiveValidCount;
    else if (liveStatus == "VALIDATION_FAILED")
        ++pumpOffLiveValidationFailureCount;
    else if (liveStatus == "INVALID_FRAME")
        ++pumpOffLiveInvalidCount;

    if (trafficChart != nullptr)
    {
        trafficChart->addDataPoint(
            scenario,
            pumpOffLiveValidCount,
            pumpOffLiveValidationFailureCount,
            pumpOffLiveInvalidCount
        );
    }
}
else if (scenario == "invalid_traffic")
{
    if (liveStatus == "VALID")
        ++invalidTrafficValidCount;
    else if (liveStatus == "VALIDATION_FAILED")
        ++invalidTrafficValidationFailureCount;
    else if (liveStatus == "INVALID_FRAME")
        ++invalidTrafficInvalidCount;

    if (trafficChart != nullptr)
    {
        trafficChart->addDataPoint(
            scenario,
            invalidTrafficValidCount,
            invalidTrafficValidationFailureCount,
            invalidTrafficInvalidCount
        );
    }
}

        // Update live Protocol Analysis counters.
        ++liveProtocolTotalFrames;

        int protocolFunctionCode =
            modbusEvent.value("function_code").toInt(-1);

        if (protocolFunctionCode == 3)
        {
            ++liveProtocolFC03Count;
        }
        else if (protocolFunctionCode == 6)
        {
            ++liveProtocolFC06Count;
        }

        QString protocolReason =
            modbusEvent.value("reason").toString();

        if (protocolReason == "UNSUPPORTED FUNCTION CODE")
        {
            ++liveProtocolUnsupportedCount;
        }

        if (protocolReason.contains("INVALID REGISTER QUANTITY"))
        {
            ++liveProtocolInvalidQuantityCount;
        }
        else if (protocolReason.contains("INVALID DATA LENGTH"))
        {
            ++liveProtocolInvalidLengthCount;
        }
        else if (protocolReason == "INVALID UNIT ID")
        {
            ++liveProtocolInvalidUnitCount;
        }
        else if (protocolReason.contains("REGISTER RANGE"))
        {
            ++liveProtocolRegisterRangeCount;
        }
        else if (protocolReason.contains("MBAP"))
        {
            ++liveProtocolInvalidMbapCount;
        }

        if (liveStatus == "VALID")
        {
            ++liveProtocolValidCount;
        }
        else if (liveStatus == "VALIDATION_FAILED")
        {
            ++liveProtocolValidationFailureCount;
        }
        else if (liveStatus == "INVALID_FRAME")
        {
            ++liveProtocolInvalidFrameCount;
        }

        // Refresh Live Security Activity panel.
        if (liveSecuritySummary != nullptr)
        {
            liveSecuritySummary->setText(
                QString(
                    "LIVE SECURITY ACTIVITY\n\n"
                    "Live Frames: %1\n"
                    "Valid Requests: %2\n"
                    "Validation Failures: %3\n"
                    "Invalid Frames: %4"
                )
                .arg(liveTotalFrames)
                .arg(liveValidCount)
                .arg(liveValidationFailureCount)
                .arg(liveInvalidFrameCount)
            );
        }

        // Refresh Live Protocol Analysis panel.
        if (liveProtocolSummary != nullptr)
        {
            liveProtocolSummary->setText(
                QString(
                    "LIVE PROTOCOL ANALYSIS\n\n"
                    "Live Frames: %1\n"
                    "FC03 Frames: %2\n"
                    "FC06 Frames: %3\n"
                    "Unsupported Functions: %4\n"
                    "Valid Frames: %5\n"
                    "Validation Failures: %6\n"
                    "Invalid Frames: %7"
                )
                .arg(liveProtocolTotalFrames)
                .arg(liveProtocolFC03Count)
                .arg(liveProtocolFC06Count)
                .arg(liveProtocolUnsupportedCount)
                .arg(liveProtocolValidCount)
                .arg(liveProtocolValidationFailureCount)
                .arg(liveProtocolInvalidFrameCount)
            );
        }

        // Refresh Live Protocol Validation Findings panel.
        if (liveProtocolFindingsSummary != nullptr)
        {
            liveProtocolFindingsSummary->setText(
                QString(
                    "LIVE VALIDATION FINDINGS\n\n"
                    "Unsupported Function Code: %1\n"
                    "Invalid Register Quantity: %2\n"
                    "Invalid Data Length: %3\n"
                    "Invalid Unit ID: %4\n"
                    "Register Range Exceeded: %5\n"
                    "Invalid MBAP Frame: %6"
                )
                .arg(liveProtocolUnsupportedCount)
                .arg(liveProtocolInvalidQuantityCount)
                .arg(liveProtocolInvalidLengthCount)
                .arg(liveProtocolInvalidUnitCount)
                .arg(liveProtocolRegisterRangeCount)
                .arg(liveProtocolInvalidMbapCount)
            );
        }

        // Update Network Traffic only when its widgets are available.
        if (trafficTable != nullptr)
        {
            int row = trafficTable->rowCount();
            trafficTable->insertRow(row);

            int frameNumber =
                modbusEvent.value("frame_number").toInt();

            int transactionId =
                modbusEvent.value("transaction_id").toInt();

            int functionCode =
                modbusEvent.value("function_code").toInt();

            QString functionCodeText =
                QString("0x%1")
                    .arg(functionCode, 2, 16, QChar('0'))
                    .toUpper();

            trafficTable->setItem(
                row, 0,
                new QTableWidgetItem(
                    QString("LIVE #%1").arg(frameNumber)
                )
            );

            trafficTable->setItem(
                row, 1,
                new QTableWidgetItem(
                    QString::number(transactionId)
                )
            );

            trafficTable->setItem(
                row, 2,
                new QTableWidgetItem(functionCodeText)
            );

            QTableWidgetItem *statusItem =
                new QTableWidgetItem(liveStatus);

            // Preserve the event for the existing frame-details view.
            statusItem->setData(
                Qt::UserRole,
                QJsonDocument(modbusEvent).toJson(
                    QJsonDocument::Compact
                )
            );

            trafficTable->setItem(row, 3, statusItem);

            if (liveStatus == "VALID")
            {
                statusItem->setBackground(QColor("#dcfce7"));
                statusItem->setForeground(QColor("#166534"));
            }
            else if (liveStatus == "VALIDATION_FAILED")
            {
                statusItem->setBackground(QColor("#fef3c7"));
                statusItem->setForeground(QColor("#92400e"));
            }
            else if (liveStatus == "INVALID_FRAME")
            {
                statusItem->setBackground(QColor("#fee2e2"));
                statusItem->setForeground(QColor("#991b1b"));
            }

            trafficTable->setItem(
                row, 4,
                new QTableWidgetItem(
                    modbusEvent.value("reason").toString()
                )
            );

            // Display historical and live statistics separately.
            if (trafficSummary != nullptr)
            {
                trafficSummary->setText(
                    QString(
                        "Historical Frames: %1    |    "
                        "Live Frames: %2    |    "
                        "Live Valid: %3    |    "
                        "Live Validation Failures: %4    |    "
                        "Live Invalid Frames: %5"
                    )
                    .arg(trafficTable->rowCount() - liveTotalFrames)
                    .arg(liveTotalFrames)
                    .arg(liveValidCount)
                    .arg(liveValidationFailureCount)
                    .arg(liveInvalidFrameCount)
                );
            }

            trafficTable->scrollToBottom();
        }
    }

    // Update scenario and connection information.
    liveScenarioLabel->setText(
        root.value("scenario_description").toString()
    );

    lastLiveSampleAt = QDateTime::currentDateTime();

    liveConnectionLabel->setText(
        QString("LIVE SIMULATION | Sample #%1")
            .arg(root.value("sequence").toInt())
    );

    // Keep the connection watchdog active.
    if (!liveDataWatchdog->isActive())
    {
        liveDataWatchdog->start();
    }
}
// Start the continuous Python live simulation
void MainWindow::startLiveSimulation()
{
    if (liveSimulationProcess == nullptr)
        return;

    if (liveSimulationProcess->state() != QProcess::NotRunning)
        stopLiveSimulation();

    QString projectRoot = QDir(
        QCoreApplication::applicationDirPath()
    ).absoluteFilePath("..");

    projectRoot = QDir::cleanPath(projectRoot);

    QString pythonExecutable = QDir(projectRoot).filePath(
        ".venv/Scripts/python.exe"
    );

    if (!QFile::exists(pythonExecutable))
    {
        liveConnectionLabel->setText(
            "ERROR: Python virtual environment not found."
        );
        return;
    }

    QString scenario =
        liveScenarioSelector->currentData().toString();

    if (scenario.isEmpty())
    {
        liveConnectionLabel->setText(
            "ERROR: No simulation scenario selected."
        );
        return;
    }

    // Reset the previous live session.
    liveOutputBuffer.clear();

    liveConnectionLabel->setText("● LIVE SIMULATION");
    liveConnectionLabel->setStyleSheet(
    "color: #16a34a;"
    "font-size: 14px;"
    "font-weight: bold;"
    );
    liveScenarioLabel->setText(
    "Selected Scenario: " + liveScenarioSelector->currentText()
    );

    tankVolumeValueLabel->setText("--");
    tankLevelValueLabel->setText("--");
    pumpStatusValueLabel->setText("OFF");
    inletFlowValueLabel->setText("--");
    outletFlowValueLabel->setText("--");
    validationValueLabel->setText("WAITING");

    tankLevelProgress->setValue(0);
    tankLevelProgress->setFormat("Waiting for data...");

    // Start the independent live decision bridge.
liveDecisionOutputBuffer.clear();

liveDecisionSummary->setText(
    "LIVE SIMULATED DECISION\n\n"
    "Decision: WAITING\n"
    "Waiting for live protocol and physics evidence."
);

liveDecisionProcess->setWorkingDirectory(projectRoot);

QStringList decisionArguments;
decisionArguments << "-m"
                  << "src.python.integration.live_decision_bridge";

if (liveDecisionProcess->state() != QProcess::NotRunning)
{
    liveDecisionProcess->kill();
    liveDecisionProcess->waitForFinished(1000);
}

liveDecisionProcess->start(
    pythonExecutable,
    decisionArguments
);

if (liveDecisionProcess->waitForStarted(1500))
{
    liveDecisionProcess->write(
        "{\"data_status\":\"SESSION_RESET\"}\n"
    );
}
else
{
    liveDecisionSummary->setText(
        "LIVE SIMULATED DECISION: BRIDGE UNAVAILABLE\n\n" +
        liveDecisionProcess->errorString()
    );
}

    // Launch Python from the project root.
    liveSimulationProcess->setWorkingDirectory(projectRoot);

    QStringList arguments;
    arguments << "-m"
              << "src.python.integration.live_simulation"
              << "--scenario"
              << scenario
              << "--interval"
              << "1";

    lastLiveSampleAt = QDateTime::currentDateTime();

    // Start watchdog immediately, including during startup.
    liveDataWatchdog->start();

    startSimulationButton->setEnabled(false);
    stopSimulationButton->setEnabled(true);

    liveSimulationProcess->start(
        pythonExecutable,
        arguments
    );
}

// Stop the live Python simulation safely

void MainWindow::stopLiveSimulation()
{
    // Stop the watchdog first.
    if (liveDataWatchdog != nullptr)
        liveDataWatchdog->stop();

    // Terminate the Python simulation process.
    if (liveSimulationProcess != nullptr &&
        liveSimulationProcess->state() != QProcess::NotRunning)
    {
        liveSimulationProcess->terminate();

        if (!liveSimulationProcess->waitForFinished(1500))
        {
            liveSimulationProcess->kill();
            liveSimulationProcess->waitForFinished(1000);
        }
    }

        // Terminate the independent live decision bridge.
    if (liveDecisionProcess != nullptr &&
        liveDecisionProcess->state() != QProcess::NotRunning)
    {
        liveDecisionProcess->terminate();

        if (!liveDecisionProcess->waitForFinished(1500))
        {
            liveDecisionProcess->kill();
            liveDecisionProcess->waitForFinished(1000);
        }
    }

    // Clear pending decision output.
    liveDecisionOutputBuffer.clear();

    // Reset the live decision panel.
    if (liveDecisionSummary != nullptr)
    {
        liveDecisionSummary->setText(
            "LIVE SIMULATED DECISION\n\n"
            "Decision: STOPPED\n"
            "Start a simulation to evaluate new evidence."
        );
    }

    // Clear remaining live output.
    liveOutputBuffer.clear();

    // Reset Physics Monitoring Data Status.
    if (physicsSnapshotInfo != nullptr)
    {
        physicsSnapshotInfo->setText(
            "Data Source: VoltGuard Live Simulation Engine\n"
            "Data Status: STOPPED"
        );
    }

    // Reset System Overview pump status.
if (pumpStatusValueLabel != nullptr)
{
    pumpStatusValueLabel->setText("OFF");
    pumpStatusValueLabel->setStyleSheet(
    "color: #dc2626; font-size: 21px; font-weight: bold; border: none; background: transparent;"
);
}
    // Reset Physics Monitoring pump status.
    if (physicsPumpValue != nullptr)
    {
        physicsPumpValue->setText("OFF");
        physicsPumpValue->setStyleSheet(
            "color: #dc2626;"
            "font-size: 25px;"
            "font-weight: bold;"
            "background-color: transparent;"
        );
    }

    // Reset validation status.
if (validationValueLabel != nullptr)
{
    validationValueLabel->setText("NOT RUN");
    validationValueLabel->setStyleSheet(
    "color: #dc2626; font-size: 21px; font-weight: bold; border: none; background: transparent;"
);
}

    // Restore simulation controls.
    if (startSimulationButton != nullptr)
        startSimulationButton->setEnabled(true);

    if (stopSimulationButton != nullptr)
        stopSimulationButton->setEnabled(false);

    // Update connection status.
    if (liveConnectionLabel != nullptr)
    {
        liveConnectionLabel->setText("SIMULATION STOPPED");

        liveConnectionLabel->setStyleSheet(
            "color: #dc2626;"
            "font-size: 14px;"
            "font-weight: bold;"
        );
    }
}

// Detect stopped processes and stale live samples
void MainWindow::updateLiveConnectionStatus()
{
    if (liveSimulationProcess == nullptr ||
        liveDataWatchdog == nullptr)
    {
        return;
    }

    if (liveSimulationProcess->state() == QProcess::NotRunning)
    {
        liveDataWatchdog->stop();

        liveConnectionLabel->setText(
            "SIMULATION STOPPED"
        );
        liveConnectionLabel->setStyleSheet(
        "color: #dc2626;"
        "font-size: 14px;"
        "font-weight: bold;"
        );

        startSimulationButton->setEnabled(true);
        stopSimulationButton->setEnabled(false);

        return;
    }

    if (!lastLiveSampleAt.isValid())
        return;

    qint64 elapsedMilliseconds =
        lastLiveSampleAt.msecsTo(QDateTime::currentDateTime());

    if (elapsedMilliseconds > 3000)
    {
        liveConnectionLabel->setText(
            "WARNING: LIVE DATA STALE"
        );
    }
}