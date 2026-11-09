#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class QProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    QProcess *analysisProcess;

    QPushButton *overviewButton;
    QPushButton *networkButton;
    QPushButton *protocolButton;
    QPushButton *physicsButton;
    QPushButton *alertsButton;
    QPushButton *reportsButton;

    void createPages();
    void createNavigation();
    void switchPage(int pageIndex);
};

#endif // MAINWINDOW_H
