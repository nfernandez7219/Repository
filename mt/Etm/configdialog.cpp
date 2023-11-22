#include <QMessageBox>
#include <QIntValidator>
#include <QLineEdit>
#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

static const char blankString[] = QT_TRANSLATE_NOOP("ConfigDialog", "N/A");

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))
{
    ui->setupUi(this);

    //ui->comboBox_2->setInsertPolicy(QComboBox::NoInsert);
    ui->usbradio->setChecked(true);
    ui->fixedradio->setChecked(true);

/*
    gdblineEdit = new QLineEdit;
    swolineEdit = new QLineEdit;
    telnetlineEdit = new QLineEdit;
    gdblineEdit->setPlaceholderText("2331");
    swolineEdit->setPlaceholderText("2332");
    telnetlineEdit->setPlaceholderText("2333");
    */

    ui->gdblineEdit->setText("2331");
    ui->swolineEdit->setText("2332");
    ui->telnetlineEdit->setText("2333");

    fillPortsParameters();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::fillPortsParameters()
{
    ui->tintcombo->addItem(QStringLiteral("SWD"));
    ui->tintcombo->addItem(QStringLiteral("JTAG"));

    ui->tdevicecombo->addItem(QStringLiteral("GD32F425RE"));
    ui->tdevicecombo->addItem(QStringLiteral("GD32F425RG"));

    ui->endiancombo->addItem(QStringLiteral("Little Endian"));
    ui->endiancombo->addItem(QStringLiteral("Big Endian"));

    ui->speedcombo->addItem(QStringLiteral("1000"));
    ui->speedcombo->addItem(QStringLiteral("1"));
    ui->speedcombo->addItem(QStringLiteral("5"));
    ui->speedcombo->addItem(QStringLiteral("10"));
    ui->speedcombo->addItem(QStringLiteral("20"));
    ui->speedcombo->addItem(QStringLiteral("30"));
    ui->speedcombo->addItem(QStringLiteral("50"));
    ui->speedcombo->addItem(QStringLiteral("100"));
    ui->speedcombo->addItem(QStringLiteral("200"));
    ui->speedcombo->addItem(QStringLiteral("300"));
    ui->speedcombo->addItem(QStringLiteral("400"));
    ui->speedcombo->addItem(QStringLiteral("500"));
    ui->speedcombo->addItem(QStringLiteral("600"));
    ui->speedcombo->addItem(QStringLiteral("750"));
    ui->speedcombo->addItem(QStringLiteral("900"));

}

void ConfigDialog::on_pushButton_2_clicked()
{
    // cancel button
    ConfigDialog::close();
}

void ConfigDialog::on_pushButton_clicked()
{
    // setup button has been pushed
    // get all dialog data here..

    QString TargetInt;
    QString TargetDev;
    QString Endian;
    QString Speed;
    QString Gdb, SwoLineEdit, TelnetLineEdit;
    QRadioButton Usb, Tcp;
    QRadioButton Auto, Adaptived, Fixed;
    bool usb;

    TargetInt = ui->tintcombo->currentText();

    Gdb = ui->gdblineEdit->text();

    usb = Usb.isChecked();  // bakit mali!
    QMessageBox::information(this,
//                             TargetInt,
                            // ui->gdblineEdit->text(),
                             Gdb,
                             "push button");

    //x = "push button";
//    m_ui-> mainwinclass::status_print(x);
//    ui->>statusbar->showMessage("Not Connected to Server!");
}

