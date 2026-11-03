#include "mainwindow.h"

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
        "• Function Code 03 - Read Holding Registers\n"
        "• Function Code 06 - Write Single Register\n\n"
        "Validation Checks:\n"
        "• MBAP header validation\n"
        "• Protocol identifier validation\n"
        "• Message length validation\n"
        "• Function code validation\n"
        "• Register address and quantity validation\n\n"
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

    QLabel *alertsInfo = new QLabel(
        "Security Analysis Indicators\n\n"
        "This section will display protocol validation failures, "
        "invalid frames, and physics-related anomalies.\n\n"
        "Important:\n"
        "An anomaly indicator is evidence requiring investigation. "
        "It does not establish malicious intent."
    );

    alertsInfo->setWordWrap(true);

    alertsInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    alertsLayout->addWidget(alertsTitle);
    alertsLayout->addWidget(alertsInfo);
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
        "These reports contain the results generated by "
        "the local simulation and analysis pipeline."
    );

    reportsInfo->setWordWrap(true);

    reportsInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    reportsLayout->addWidget(reportsTitle);
    reportsLayout->addWidget(reportsInfo);
    reportsLayout->addStretch();

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