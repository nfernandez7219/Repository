#include <QMessageBox>
#include <QGridLayout>
#include <QString>
#include <QTextBrowser>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_settings(new SettingsDialog(this))
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
}

MainWindow::~MainWindow()
{
    delete m_ui;
}


void MainWindow::on_actionSave_Test_Results_triggered()
{
    QMessageBox::information(this,
                             "Report",
                             "Save Results");
}

void MainWindow::on_actionDisConnect_Device_triggered()
{
    QMessageBox::information(this,
                             "Report",
                             "BDisConnect");
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect_Device_2, &QAction::triggered, m_settings, &SettingsDialog::show);
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

void MainWindow::on_pushButton_clicked()
{
    QString XMAX=m_ui->adc101_avvalue->text();


    QMessageBox::information(this,
                             "Report",
                             XMAX);

}

void MainWindow::log_msg(QString string)
{

    progressval = 90;
    m_ui->progressBar->setValue(progressval);


    m_ui->logBrowser->append(string);
    m_ui->logBrowser->setLineWrapMode(QTextEdit::NoWrap);


    //QScrollBar *scrollbar = m_ui->logBrowser->verticalScrollBar();
    //bool scrollbarAtBottom  = (scrollbar->value() >= (scrollbar->maximum() - 4));
    //int scrollbarPrevValue = scrollbar->value();
}
