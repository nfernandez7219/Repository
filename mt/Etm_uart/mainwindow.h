#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextBrowser>


QT_BEGIN_NAMESPACE


namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSave_Test_Results_triggered();

    void on_actionDisConnect_Device_triggered();

    void on_pushButton_clicked();

private:
    void initActionsConnections();
    void set_tree_widget();
    void log_msg(QString);

private:
    QTextBrowser logBrowser;
    //QScrollBar   verticalScrollBar;
    int          progressval;

private:
    Ui::MainWindow *m_ui = nullptr;
    SettingsDialog *m_settings = nullptr;
};
#endif // MAINWINDOW_H
