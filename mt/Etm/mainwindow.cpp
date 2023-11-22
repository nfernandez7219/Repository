#include <QMessageBox>
#include <QGridLayout>
#include <QString>
#include <QTextBrowser>
#include <QFileDialog>
#include <QTextStream>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_settings(new ConfigDialog(this))
{
    m_ui->setupUi(this);

    initActionsConnections();

    set_tree_widget();

    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");
    log_msg("test log1");
    log_msg("test log2");
    log_msg("test log3");

    status_print("Not Connected to Server");
    //m_ui->statusbar->showMessage("Not Connected to Server!");
}

MainWindow::~MainWindow()
{
    delete m_ui;
}


void MainWindow::on_actionSave_Test_Results_triggered()
{
    QMessageBox::information(this,
                             "test result",
                             "Save Results");
}

void MainWindow::on_actionDisConnect_Device_triggered()
{
    QMessageBox::information(this,
                             "disconnect",
                             "BDisConnect");
}

void MainWindow::initActionsConnections()
{
    // connect device
    connect(m_ui->actionConnect_Device_2, &QAction::triggered, m_settings, &ConfigDialog::show);


}


void MainWindow::set_tree_widget()
{
    QTreeWidgetItem *Children;
    QTreeWidgetItem *Children2;

    Children = new QTreeWidgetItem(m_ui->treeWidget->topLevelItem(0));
    Children->setText(0,"ADC Module");
    Children2 = new QTreeWidgetItem(Children);
    Children2->setText(0,"ADC Test Item 1");

    Children = new QTreeWidgetItem(m_ui->treeWidget->topLevelItem(0));
    Children->setText(0,"DAC Module");
    Children2 = new QTreeWidgetItem(Children);
    Children2->setText(0,"DAC Test Item 1");

    Children = new QTreeWidgetItem(m_ui->treeWidget->topLevelItem(0));
    Children->setText(0,"PLM Module");
    Children2 = new QTreeWidgetItem(Children);
    Children2->setText(0,"PLM Test Item 1");

}
#if 0
void MainWindow::on_pushButton_clicked()
{
    // use chapter 3 GDB server startup using parameter input from dialog.
    // use client-server socket next..

    // client should be used here..
    //


    // Setup Button has been pressed
    QString XMAX = m_ui->adc101_avvalue->text();


    QMessageBox::information(this,
                             "push button clicked",
                             XMAX);

/*    m_ui->

    status_print("Connected to Server!");
    m_settings->close();

    // Saved all the information
    // Connect Client to Server using all the information
*/
}
#endif
void MainWindow::status_print(QString string)
{
    m_ui->statusbar->setStyleSheet("color: blue");
    m_ui->statusbar->showMessage(string);
}

void MainWindow::log_msg(QString string)
{

    progressval = 90;
    m_ui->progressBar->setValue(progressval);
    //m_ui->progressBar->hide();


    m_ui->logBrowser->append(string);
    m_ui->logBrowser->setLineWrapMode(QTextEdit::NoWrap);


    //QScrollBar *scrollbar = m_ui->logBrowser->verticalScrollBar();
    //bool scrollbarAtBottom  = (scrollbar->value() >= (scrollbar->maximum() - 4));
    //int scrollbarPrevValue = scrollbar->value();
}

void MainWindow::on_actionDownload_Diagnostic_Firmware_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a File", QDir::homePath());
    QFile ReportFile(filename);
    QTextStream Data(&ReportFile);

    if (ReportFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        Data << 0x30;
    }
/*        QMessageBox::information(this, "..",filename);
        QString text;
        QMessageBox box;
        box.setText(filename);

    QMessageBox::information(this,
                             "diag fw",
                             "Diagnotic");
*/
}
