#include "mainwindow.h"

#include <QProgressBar>
#include <QProcess>
#include <QComboBox>
#include <QStringList>
#include <QJsonArray>
#include <QList>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QComboBox>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(nullptr),
      analysisProcess(nullptr),
      overviewButton(nullptr),
      networkButton(nullptr),
      protocolButton(nullptr),
      physicsButton(nullptr),
      alertsButton(nullptr),
      reportsButton(nullptr)
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


    // Stacked pages
    stackedWidget = new QStackedWidget(centralWidget);

    stackedWidget->setMinimumWidth(500);
    stackedWidget->setStyleSheet(
    "QStackedWidget { background-color: #f1f5f9; }"
);

    analysisProcess = new QProcess(this);

    createPages();

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
    mainLayout->addWidget(stackedWidget, 1);

    switchPage(0);
}

MainWindow::~MainWindow() = default;


// Create dashboard pages

void MainWindow::createPages()
{
    // =========================================
    // PAGE 1: SYSTEM OVERVIEW
    // =========================================

    QWidget *overviewPage = new QWidget(stackedWidget);
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewPage);

    QLabel *overviewTitle = new QLabel("System Overview");
    overviewTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *overviewSubtitle = new QLabel(
        "Physics-Aware ICS/SCADA Intrusion Detection System"
    );

    overviewSubtitle->setStyleSheet(
        "color: #64748b;"
        "font-size: 14px;"
        "background-color: transparent;"
    );

    overviewLayout->addWidget(overviewTitle);
    overviewLayout->addWidget(overviewSubtitle);

    // -----------------------------------------
    // Read Combined JSON Report
    // -----------------------------------------

    QString reportPath = QDir(
    QCoreApplication::applicationDirPath()
    ).absoluteFilePath("../reports/combined_analysis_report.json");

    reportPath = QDir::cleanPath(reportPath);

    QString totalFrames = "N/A";
    QString validRequests = "N/A";
    QString protocolAlerts = "N/A";
    QString physicsStatus = "N/A";
    QString overallStatus = "REPORT NOT LOADED";

    QFile ReportFile(reportPath);

    if (ReportFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QJsonParseError parseError;

        QJsonDocument document = QJsonDocument::fromJson(
            ReportFile.readAll(),
            &parseError
        );

        ReportFile.close();

        if (parseError.error == QJsonParseError::NoError
            && document.isObject())
        {
            QJsonObject root = document.object();

            QJsonObject parser =
                root.value("parser_analysis").toObject();

            QJsonObject summary =
                parser.value("summary").toObject();

            QJsonObject physics =
                root.value("physics_analysis").toObject();

            QJsonObject validation =
                physics.value("validation_result").toObject();

            if (summary.contains("total_frames")
                && summary.contains("valid_requests")
                && summary.contains("validation_failures")
                && summary.contains("invalid_frames")
                && validation.contains("valid"))
            {
                int total =
                    summary.value("total_frames").toInt();

                int valid =
                    summary.value("valid_requests").toInt();

                int failures =
                    summary.value("validation_failures").toInt();

                int invalid =
                    summary.value("invalid_frames").toInt();

                bool physicsValid =
                    validation.value("valid").toBool();

                totalFrames = QString::number(total);
                validRequests = QString::number(valid);

                protocolAlerts =
                    QString::number(failures + invalid);

                physicsStatus =
                    physicsValid ? "VALID" : "ANOMALY";

                overallStatus =
                    root.value("overall_status").toString();
            }
        }
    }

    // -----------------------------------------
    // Dashboard Card Creation Function
    // -----------------------------------------

    QGridLayout *grid = new QGridLayout();

    auto createCard = [](
        const QString &title,
        const QString &value,
        const QString &description,
        const QString &color
    )
    {
        QFrame *card = new QFrame();

        card->setMinimumHeight(140);

        card->setStyleSheet(
            "QFrame {"
            "background-color: white;"
            "border-radius: 10px;"
            "border: 1px solid #e2e8f0;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(card);

        QLabel *titleLabel = new QLabel(title);
        QLabel *valueLabel = new QLabel(value);
        QLabel *descriptionLabel = new QLabel(description);

        titleLabel->setStyleSheet(
            "color: #64748b;"
            "background-color: transparent;"
            "border: none;"
        );

        valueLabel->setStyleSheet(
            "color: " + color +
            "; font-size: 27px;"
            "font-weight: bold;"
            "background-color: transparent;"
            "border: none;"
        );

        descriptionLabel->setStyleSheet(
            "color: #94a3b8;"
            "background-color: transparent;"
            "border: none;"
        );

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(descriptionLabel);
        layout->addStretch();

        return card;
    };

    // -----------------------------------------
    // Add Dashboard Cards
    // -----------------------------------------

    grid->addWidget(
        createCard(
            "Protocol Frames",
            totalFrames,
            "Total frames processed",
            "#2563eb"
        ),
        0, 0
    );

    grid->addWidget(
        createCard(
            "Valid Requests",
            validRequests,
            "Requests passing validation",
            "#16a34a"
        ),
        0, 1
    );

    grid->addWidget(
        createCard(
            "Protocol Alerts",
            protocolAlerts,
            "Validation failures and invalid frames",
            "#dc2626"
        ),
        1, 0
    );

    grid->addWidget(
        createCard(
            "Physics Status",
            physicsStatus,
            "Process operating-condition validation",
            physicsStatus == "VALID" ? "#16a34a" : "#dc2626"
        ),
        1, 1
    );

    overviewLayout->addLayout(grid);

    // Overall Analysis Status
    QLabel *statusLabel = new QLabel(
        "Overall Analysis Status: " + overallStatus
    );

    statusLabel->setWordWrap(true);

    statusLabel->setStyleSheet(
        "background-color: white;"
        "padding: 15px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "color: #334155;"
    );

    overviewLayout->addWidget(statusLabel);
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

    QLabel *networkInfo = new QLabel(
        "Synthetic Modbus/TCP Traffic Monitoring\n\n"
        "Traffic Source: data/generated/modbus_traffic.hex\n"
        "Environment: LOCAL_TEST\n\n"
        "This page is prepared for displaying generated "
        "industrial protocol traffic."
    );

    networkInfo->setWordWrap(true);

    networkInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    networkLayout->addWidget(networkTitle);
    networkLayout->addWidget(networkInfo);
    networkLayout->addStretch();

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

    QLabel *protocolInfo = new QLabel(
        "Modbus/TCP Protocol Analysis\n\n"
        "Supported Functions:\n"
        "â€¢ Function Code 03 - Read Holding Registers\n"
        "â€¢ Function Code 06 - Write Single Register\n\n"
        "Validation Checks:\n"
        "â€¢ MBAP header validation\n"
        "â€¢ Protocol identifier validation\n"
        "â€¢ Message length validation\n"
        "â€¢ Function code validation\n"
        "â€¢ Register address and quantity validation\n\n"
        "Detailed parser results are available in "
        "data/generated/parser_results.json."
    );

    protocolInfo->setWordWrap(true);

    protocolInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    protocolLayout->addWidget(protocolTitle);
    protocolLayout->addWidget(protocolInfo);
    protocolLayout->addStretch();

    stackedWidget->addWidget(protocolPage);

    // =========================================
    // PAGE 4: PHYSICS MONITORING
    // =========================================

    QWidget *physicsPage = new QWidget(stackedWidget);
    QVBoxLayout *physicsLayout = new QVBoxLayout(physicsPage);

    QLabel *physicsTitle = new QLabel("Physics Monitoring");

    physicsTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *physicsSubtitle = new QLabel(
        "Synthetic Water Tank Process Monitoring"
    );

    physicsSubtitle->setStyleSheet(
        "color: #64748b;"
        "font-size: 14px;"
        "background-color: transparent;"
    );

    // Scenario Selector

    QLabel *scenarioLabel = new QLabel("Select Test Scenario:");

    QComboBox *scenarioSelector = new QComboBox();

    scenarioSelector->addItem("Normal Scenario", "process_snapshot.json");
    scenarioSelector->addItem("Abnormal Scenario", "abnormal_process_snapshot.json");

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

    auto createPhysicsCard = [](
        const QString &title,
        const QString &color
    )
    {
        QFrame *card = new QFrame();

        card->setMinimumHeight(110);

        card->setStyleSheet(
            "QFrame {"
            "background-color: white;"
            "border: 1px solid #e2e8f0;"
            "border-radius: 10px;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(card);

        QLabel *titleLabel = new QLabel(title);
        QLabel *valueLabel = new QLabel("N/A");

        titleLabel->setStyleSheet(
            "color: #64748b;"
            "font-size: 13px;"
            "border: none;"
            "background-color: transparent;"
        );

        valueLabel->setStyleSheet(
            "color: " + color +
            "; font-size: 25px;"
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

    QLabel *tankValue = qobject_cast<QLabel *>(
        tankCard->property("valueLabel").value<QObject *>()
    );

    QLabel *pumpValue = qobject_cast<QLabel *>(
        pumpCard->property("valueLabel").value<QObject *>()
    );

    QLabel *valveValue = qobject_cast<QLabel *>(
        valveCard->property("valueLabel").value<QObject *>()
    );

    QLabel *flowValue = qobject_cast<QLabel *>(
        flowCard->property("valueLabel").value<QObject *>()
    );

    physicsGrid->addWidget(tankCard, 0, 0);
    physicsGrid->addWidget(pumpCard, 0, 1);
    physicsGrid->addWidget(valveCard, 1, 0);
    physicsGrid->addWidget(flowCard, 1, 1);

    // Tank Level Visualization

    QFrame *tankVisualizationCard = new QFrame();

    tankVisualizationCard->setMinimumHeight(220);

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

    QLabel *tankVisualizationValue =
        new QLabel("Tank Level: N/A");

    tankVisualizationValue->setAlignment(Qt::AlignCenter);

    tankVisualizationValue->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2563eb;"
        "border: none;"
        "background-color: transparent;"
    );

    QProgressBar *tankLevelBar = new QProgressBar();

    tankLevelBar->setOrientation(Qt::Vertical);
    tankLevelBar->setRange(0, 100);
    tankLevelBar->setValue(0);
    tankLevelBar->setTextVisible(true);
    tankLevelBar->setFormat("%p%");
    tankLevelBar->setMinimumHeight(130);
    tankLevelBar->setMaximumWidth(100);
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

    tankVisualizationLayout->addWidget(
        tankVisualizationValue
    );

    physicsLayout->addWidget(tankVisualizationCard);


    // Snapshot Information

    QLabel *snapshotInfo = new QLabel();
    QLabel *physicsStatusLabel = new QLabel();

    physicsStatusLabel->setWordWrap(true);

    physicsStatusLabel->setStyleSheet(
        "background-color: white;"
        "padding: 14px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "color: #334155;"
        "font-weight: bold;"
    );

    // Physics Validation Results

    QLabel *validationTitle = new QLabel(
        "Physics Validation Results"
    );

    validationTitle->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *validationSummary = new QLabel();
    QLabel *violationLabel = new QLabel();
    QLabel *warningLabel = new QLabel();

    validationSummary->setWordWrap(true);
    violationLabel->setWordWrap(true);
    warningLabel->setWordWrap(true);

    validationSummary->setStyleSheet(
        "background-color: white;"
        "padding: 12px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "color: #334155;"
        "font-weight: bold;"
    );

    violationLabel->setStyleSheet(
        "background-color: white;"
        "padding: 12px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "color: #dc2626;"
    );

    warningLabel->setStyleSheet(
        "background-color: white;"
        "padding: 12px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "color: #b45309;"
    );

    // Dynamic Scenario Loading

    auto updatePhysicsScenario = [
        tankValue,
        pumpValue,
        valveValue,
        flowValue,
        snapshotInfo,
        physicsStatusLabel,
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

        physicsStatusLabel->setText("Data Status: LOADING");

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

                    bool pump =
                        process.value("pump_on").toBool();

                    double valve =
                        process.value("valve_opening_percent").toDouble();

                    double flow =
                        process.value("flow_rate_lpm").toDouble();

                    tankValue->setText(
                        QString::number(level, 'f', 1) + "%"
                    );

                    pumpValue->setText(pump ? "ON" : "OFF");

                    pumpValue->setStyleSheet(
                        "color: " + QString(pump ? "#16a34a" : "#dc2626") +
                        "; font-size: 25px; font-weight: bold;"
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
            physicsStatusLabel->setText("Data Status: SNAPSHOT LOADED");
            snapshotInfo->setText(
                "Data Source: " + selectedFile +
                "\nEnvironment: LOCAL_TEST" +
                "\nTransaction ID: " + snapshotTransaction
            );
        }
        else
        {
            physicsStatusLabel->setText(
                "Data Status: SNAPSHOT COULD NOT BE LOADED"
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

    physicsLayout->addWidget(physicsTitle);
    physicsLayout->addWidget(physicsSubtitle);
    physicsLayout->addSpacing(10);
    physicsLayout->addLayout(scenarioLayout);
    physicsLayout->addSpacing(10);
    physicsLayout->addLayout(physicsGrid);
    physicsLayout->addWidget(snapshotInfo);
    physicsLayout->addWidget(physicsStatusLabel);
    physicsLayout->addWidget(validationTitle);
    physicsLayout->addWidget(validationSummary);
    physicsLayout->addWidget(violationLabel);
    physicsLayout->addWidget(warningLabel);

    stackedWidget->addWidget(physicsPage);

    // =========================================
    // PAGE 5: SECURITY ALERTS
    // =========================================

    QWidget *alertsPage = new QWidget(stackedWidget);
    QVBoxLayout *alertsLayout = new QVBoxLayout(alertsPage);

    QLabel *alertsTitle = new QLabel("Security Alerts");

    alertsTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *scenarioInfo = new QLabel();
    QLabel *protocolSummary = new QLabel();
    QLabel *protocolDetails = new QLabel();
    QLabel *physicsSummary = new QLabel();
    QLabel *decisionSummary = new QLabel();
    QLabel *decisionDetails = new QLabel();
    QLabel *physicsDetails = new QLabel();
    QLabel *interpretationInfo = new QLabel();

    scenarioInfo->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #334155;"
    );

    protocolSummary->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #1e293b;"
    );

    physicsSummary->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #1e293b;"
    );

    protocolDetails->setStyleSheet(
        "font-size: 14px;"
        "color: #334155;"
    );

    physicsDetails->setStyleSheet(
        "font-size: 14px;"
        "color: #334155;"
    );

    decisionSummary->setStyleSheet(
    "font-size: 19px;"
    "font-weight: bold;"
    "color: #b91c1c;"
    );

    decisionDetails->setStyleSheet(
    "font-size: 14px;"
    "color: #334155;"
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

        physicsDetails->setText(
            "Violations: " + QString::number(violations.size()) + "\n"
            "Warnings: " + QString::number(warnings.size())
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
    alertsLayout->addWidget(scenarioInfo);
    alertsLayout->addSpacing(8);
    alertsLayout->addWidget(protocolSummary);
    alertsLayout->addWidget(protocolDetails);
    alertsLayout->addSpacing(8);
    alertsLayout->addWidget(physicsSummary);
    alertsLayout->addWidget(physicsDetails);
    alertsLayout->addSpacing(8);
    alertsLayout->addWidget(decisionSummary);
    alertsLayout->addWidget(decisionDetails);
    alertsLayout->addSpacing(8);
    alertsLayout->addWidget(interpretationInfo);
    alertsLayout->addStretch();

    stackedWidget->addWidget(alertsPage);

    // =========================================
    // PAGE 6: REPORTS
    // =========================================

    QWidget *reportsPage = new QWidget(stackedWidget);
    QVBoxLayout *reportsLayout = new QVBoxLayout(reportsPage);

    QLabel *reportsTitle = new QLabel("Reports");

    reportsTitle->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #0f172a;"
        "background-color: transparent;"
    );

    QLabel *reportsInfo = new QLabel(
        "VoltGuard Analysis Reports\n\n"
        "Combined Analysis Report:\n"
        "reports/combined_analysis_report.json\n\n"
        "Protocol Parser Report:\n"
        "data/generated/parser_results.json\n\n"
        "Run a scenario to generate fresh analysis results."
    );

    reportsInfo->setWordWrap(true);

    reportsInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    QPushButton *runAnalysisButton =
        new QPushButton("Run Analysis");

    runAnalysisButton->setMinimumHeight(42);
    runAnalysisButton->setMinimumWidth(180);
    runAnalysisButton->setMaximumWidth(250);

    runAnalysisButton->setStyleSheet(
    "QPushButton {"
    "color: white;"
    "background-color: #2563eb;"
    "border: 1px solid #1d4ed8;"
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
    "color: white;"
    "}"
    );

    QLabel *analysisStatusLabel =
        new QLabel("Analysis Status: READY");

    analysisStatusLabel->setWordWrap(true);

    analysisStatusLabel->setStyleSheet(
        "color: #334155;"
        "font-size: 14px;"
        "background-color: transparent;"
    );

    reportsLayout->addWidget(reportsTitle);
    reportsLayout->addWidget(reportsInfo);
    reportsLayout->addSpacing(15);
    reportsLayout->addWidget(runAnalysisButton);
    reportsLayout->addWidget(analysisStatusLabel);
    reportsLayout->addStretch();

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
                return;
            }

            if (!QFile::exists(scriptPath))
            {
                analysisStatusLabel->setText(
                    "Analysis Status: run_analysis.py not found."
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

                updatePhysicsScenario();
                updateSecurityAlerts();
            }
            else
            {
                analysisStatusLabel->setText(
                    "Analysis Status: FAILED.\n"
                    "Exit code: " + QString::number(exitCode)
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
            runAnalysisButton->setEnabled(true);

            analysisStatusLabel->setText(
                "Analysis Status: PROCESS ERROR.\n" +
                analysisProcess->errorString()
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
    {
        return;
    }

    if (pageIndex < 0 || pageIndex >= stackedWidget->count())
    {
        return;
    }

    stackedWidget->setCurrentIndex(pageIndex);
}
