#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QUdpSocket>

#include <string>

#include "exception.h"
#include "robot_state.h"

class UDPReceiver : public QObject
{
    Q_OBJECT

public:
    UDPReceiver() : QObject(0) {}

    virtual ~UDPReceiver()
    {
        if (socket_)
            delete socket_;
    }

    void initUDPClient(int port)
    {
        socket_ = new QUdpSocket(this);
        bool result = socket_->bind(QHostAddress::Any, port);
        if (!result)
        {
            qDebug() << "UDPReceiver::initUDPClient(): UDP socket binding error.";
            throw Exception("UDPReceiver::initUDPClient()", "UDP socket binding error.");
        }

        robot_port_ = port;

        connect(socket_, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    }

    void sendMessage(QString ip_addr, int port, QString msg)
    {
        QHostAddress addr = QHostAddress(ip_addr);
        QByteArray datagram(msg.toStdString().c_str());
        socket_->writeDatagram(datagram, addr, port);
        qDebug() << "Sending UDP message [" << ip_addr << ", " << port << "]: " << msg;
    }

protected:
    template<typename T>
    T get(QDataStream &stream)
    {
        T value;
        stream >> value;
        return value;
    }

protected:
    int robot_port_{0};
    QUdpSocket *socket_{nullptr};

private slots:
    void readPendingDatagrams()
    {
        QHostAddress sender;
        quint16 sender_port;

        while (socket_->hasPendingDatagrams())
        {
            QByteArray buf(socket_->pendingDatagramSize(), Qt::Uninitialized);
            QDataStream stream(&buf, QIODevice::ReadOnly);
            socket_->readDatagram(buf.data(), buf.size(), &sender, &sender_port);
            qDebug() << "Received UDP message from: " << sender << ":" << sender_port;

            if (buf.size() >= 4)
            {
                RobotState state;
                state.emergency_on_ = get<bool>(stream);
                state.turtle_mode_on_ = get<bool>(stream);
                state.reverse_mode_on_ = get<bool>(stream);
                state.battery_level_ = get<quint8>(stream);

                emit robotStateReceived(state);
            }
        }
    }

signals:
    void robotStateReceived(RobotState robot_state);
};

#endif // UDP_RECEIVER_H