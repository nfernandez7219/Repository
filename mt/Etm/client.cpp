#include "client.h"

static inline QByteArray IntToArray(qint32 source);

client::client(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readRead()));
}

void client::readyRead()
{
    qDebug() << "Reading...";
    qDebug() << socket->readAll();
}

void client::Connect()
{
    socket = new QTcpSocket(this);
    socket->connectToHost("bogotobogo.com", 80);

    if (socket->waitForConnected(3000))
    {
        qDebug() << "Connected";

        // send
        socket->write("hello server\r\n\r\n\r\n\r\n");
        socket->waitForBytesWritten(1000);
        socket->waitForReadyRead(3000);

        qDebug() << "Reading: " << socket->bytesAvailable();

        qDebug() << socket->readAll();

        socket->close();
    }
    else
    {
        qDebug() << "Not connected!";
    }
    // sent
    // got
    // closed
}

bool client::connectToHost(QString host)
{
    socket->connectToHost(host, 1024);
    return socket->waitForConnected();
}

bool client::writeData(QByteArray data)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        //socket->write(IntToArray(data.size())); // write size of data
        socket->write(data);    // write the data itself
        return socket->waitForBytesWritten();
    }
    else
        return false;
}

QByteArray IntToArray(qint32 source)
{
    // avoid use of cast, this is the Qt way to serialize objects

    QByteArray temp;
    /*
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    */
    return temp;
}
