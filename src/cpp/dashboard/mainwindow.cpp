#include "mainwindow.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QWidget>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    QFrame *sidebar = new QFrame();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("background-color: #0f172a;");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

    QLabel *logo = new QLabel("VOLTGUARD");
    logo->setStyleSheet(
        "color: white; font-size: 22px; font-weight: bold;"
    );

    sidebarLayout->addWidget(logo);
    sidebarLayout->addSpacing(30);

    QStringList menuItems = {
        "System Overview",
        "Network Traffic",
        "Protocol Analysis",
        "Physics Monitoring",
        "Security Alerts",
        "Reports"
    };

    for (const QString &item : menuItems) {
        QPushButton *button = new QPushButton(item);
        button->setStyleSheet(
            "QPushButton {"
            "color: #cbd5e1;"
            "background-color: transparent;"
            "border: none;"
            "text-align: left;"
            "padding: 12px;"
            "font-size: 13px;"
            "}"
            "QPushButton:hover {"
            "background-color: #1e293b;"
            "color: white;"
            "}"
        );

        sidebarLayout->addWidget(button);
    }

    sidebarLayout->addStretch();

    // Main content
    QWidget *content = new QWidget();

    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(30, 25, 30, 25);
    contentLayout->setSpacing(20);

    QLabel *heading = new QLabel("System Overview");
    heading->setStyleSheet(
        "font-size: 26px; font-weight: bold; color: #0f172a;"
    );

    QLabel *subtitle = new QLabel(
        "Physics-Aware ICS/SCADA Intrusion Detection System"
    );

    subtitle->setStyleSheet(
        "font-size: 13px; color: #64748b;"
    );

    contentLayout->addWidget(heading);
    contentLayout->addWidget(subtitle);

    // Report path: project-root/reports/combined_analysis_report.json
    QString reportPath = QDir(
        QCoreApplication::applicationDirPath()
    ).absoluteFilePath("../reports/combined_analysis_report.json");

    reportPath = QDir::cleanPath(reportPath);

    QString protocolFrames = "N/A";
    QString validRequests = "N/A";
    QString protocolAlerts = "N/A";
    QString physicsStatus = "N/A";
    QString overallStatus = "REPORT NOT LOADED";
    QString reportMessage = "Unable to load analysis report.";

    QFile reportFile(reportPath);

    if (reportFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(
            reportFile.readAll(),
            &parseError
        );

        reportFile.close();

        if (parseError.error == QJsonParseError::NoError
            && document.isObject()) {

            QJsonObject root = document.object();

            QJsonObject parserAnalysis =
                root.value("parser_analysis").toObject();

            QJsonObject summary =
                parserAnalysis.value("summary").toObject();

            QJsonObject physicsAnalysis =
                root.value("physics_analysis").toObject();

            QJsonObject validation =
                physicsAnalysis.value("validation_result").toObject();

            if (root.contains("parser_analysis")
                && root.contains("physics_analysis")
                && root.contains("overall_status")
                && summary.contains("total_frames")
                && summary.contains("valid_requests")
                && summary.contains("validation_failures")
                && summary.contains("invalid_frames")
                && validation.contains("valid")) {

                int totalFrames =
                    summary.value("total_frames").toInt();

                int validCount =
                    summary.value("valid_requests").toInt();

                int validationFailures =
                    summary.value("validation_failures").toInt();

                int invalidFrames =
                    summary.value("invalid_frames").toInt();

                bool physicsValid =
                    validation.value("valid").toBool();

                protocolFrames = QString::number(totalFrames);
                validRequests = QString::number(validCount);

                protocolAlerts = QString::number(
                    validationFailures + invalidFrames
                );

                physicsStatus = physicsValid ? "VALID" : "ANOMALY";

                overallStatus = root.value(
                    "overall_status"
                ).toString();

                reportMessage = "JSON analysis report loaded successfully.";
            }
            else {
                reportMessage = "Report is missing required fields.";
            }
        }
        else {
            reportMessage = "Invalid JSON: " + parseError.errorString();
        }
    }
    else {
        reportMessage = "Cannot open report: " + reportPath;
    }

    // Dashboard card creation
    auto createCard = [](
        const QString &title,
        const QString &value,
        const QString &description,
        const QString &color
    ) {
        QFrame *card = new QFrame();

        card->setMinimumHeight(145);

        card->setStyleSheet(
            "QFrame {"
            "background-color: white;"
            "border-radius: 10px;"
            "border: 1px solid #e2e8f0;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 15, 18, 15);

        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet(
            "color: #64748b; font-size: 13px; border: none;"
        );

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(
            "color: " + color +
            "; font-size: 27px; font-weight: bold; border: none;"
        );

        QLabel *descriptionLabel = new QLabel(description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet(
            "color: #94a3b8; font-size: 11px; border: none;"
        );

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(descriptionLabel);
        layout->addStretch();

        return card;
    };

    // Cards populated from the JSON report
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(18);

    grid->addWidget(
        createCard(
            "Protocol Frames",
            protocolFrames,
            "Total frames processed",
            "#2563eb"
        ),
        0, 0
    );

    grid->addWidget(
        createCard(
            "Valid Requests",
            validRequests,
            "Requests passing protocol validation",
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

    contentLayout->addLayout(grid);

    QLabel *overallLabel = new QLabel(
        "Overall Analysis Status: " + overallStatus
    );

    overallLabel->setWordWrap(true);
    overallLabel->setStyleSheet(
        "background-color: white;"
        "color: #334155;"
        "padding: 15px;"
        "border: 1px solid #e2e8f0;"
        "border-radius: 8px;"
        "font-size: 13px;"
        "font-weight: bold;"
    );

    contentLayout->addWidget(overallLabel);

    QLabel *reportLabel = new QLabel(reportMessage);
    reportLabel->setWordWrap(true);
    reportLabel->setStyleSheet(
        "color: #64748b; font-size: 11px;"
    );

    contentLayout->addWidget(reportLabel);
    contentLayout->addStretch();

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(content);
}

MainWindow::~MainWindow() = default;