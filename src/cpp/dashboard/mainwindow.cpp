#include "mainwindow.h"

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
#include <QDebug>

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

    QFile reportFile(reportPath);

    if (reportFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QJsonParseError parseError;

        QJsonDocument document = QJsonDocument::fromJson(
            reportFile.readAll(),
            &parseError
        );

        reportFile.close();

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

    QLabel *physicsInfo = new QLabel(
        "Synthetic Water Tank Process Model\n\n"
        "Monitored Variables:\n"
        "• Tank level percentage\n"
        "• Pump operating state\n"
        "• Valve opening percentage\n"
        "• Flow rate in litres per minute\n\n"
        "Physics validation checks whether the synthetic "
        "process conditions satisfy the configured "
        "operating rules.\n\n"
        "Data Source: combined_analysis_report.json"
    );

    physicsInfo->setWordWrap(true);

    physicsInfo->setStyleSheet(
        "color: #334155;"
        "font-size: 15px;"
        "background-color: transparent;"
    );

    physicsLayout->addWidget(physicsTitle);
    physicsLayout->addWidget(physicsInfo);
    physicsLayout->addStretch();

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

  
    // -----------------------------------------
    // Diagnostic Output
    // -----------------------------------------

    qDebug() << "Total dashboard pages:" << stackedWidget->count();
    qDebug() << "Initial current page:" << stackedWidget->currentIndex();
}

// Switch between dashboard pages
void MainWindow::switchPage(int pageIndex)
{
    qDebug() << "Navigation clicked. Requested page:" << pageIndex;

    if (stackedWidget == nullptr)
    {
        qDebug() << "ERROR: Stacked widget is null.";
        return;
    }

    qDebug() << "Total registered pages:" << stackedWidget->count();

    if (pageIndex >= 0 && pageIndex < stackedWidget->count())
    {
        stackedWidget->setCurrentIndex(pageIndex);

        qDebug() << "Current page:" << stackedWidget->currentIndex();
    }
    else
    {
        qDebug() << "ERROR: Invalid page index.";
    }
}