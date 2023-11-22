#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
//#include <QtNetwork>

class client : public QObject
{
    Q_OBJECT
public:
    explicit client(QObject *parent = 0);

    void Connect();

public slots:
    bool connectToHost(QString host);
    bool writeData(QByteArray data);
    void readyRead();

private:
    QTcpSocket *socket;
};

#endif // CLIENT_H
