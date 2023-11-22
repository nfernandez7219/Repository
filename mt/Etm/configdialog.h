#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>

QT_BEGIN_NAMESPACE

namespace Ui {
class ConfigDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        //QSerialPort::DataBits dataBits;
        QString stringDataBits;
        //QSerialPort::Parity parity;
        QString stringParity;
        //QSerialPort::StopBits stopBits;
        QString stringStopBits;
        //QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    Settings settings() const;
/*
private:
    QLineEdit *gdblineEdit;
    QLineEdit *swolineEdit;
    QLineEdit *telnetlineEdit;
    */
private slots:
//    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    void fillPortsParameters();

private:
    Ui::ConfigDialog    *ui = nullptr;
    Settings            m_currentSettings;
    QIntValidator       *m_intValidator = nullptr;
};

#endif // CONFIGDIALOG_H
