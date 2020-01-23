#include "nethost.h"

NetHost::NetHost() : m_timeout(1000) {}

NetHost::~NetHost() {}

bool NetHost::bind(const AddrPair &addrPair) {
    m_addrPair = addrPair;
    if (!m_socket.bind(m_addrPair.myAddr, m_addrPair.myPort)) {
        m_errorString = tr("Cannot bind to %1:%2")
                            .arg(m_addrPair.myAddr.toString())
                            .arg(QString::number(m_addrPair.myPort));
        return false;
    }
    return true;
}

bool NetHost::isBound() const {
    return (m_socket.state() == QAbstractSocket::BoundState);
}

void NetHost::close() { m_socket.abort(); }

bool NetHost::sendPacket(const QByteArray &packet) {
    qint64 size = packet.size();
    qint64 gotSize =
        m_socket.writeDatagram(packet, m_addrPair.c64Addr, m_addrPair.c64Port);
    if (gotSize != size) {
        m_errorString = tr("Datagram wrote error (has %1 but got %2 bytes)")
                            .arg(QString::number(size))
                            .arg(QString::number(gotSize));
        return false;
    }
    emit sentPacket(size);
    return true;
}

bool NetHost::waitForPacket(int timeout) {
    return m_socket.waitForReadyRead(timeout);
}

bool NetHost::hasPackets() { return m_socket.hasPendingDatagrams(); }

bool NetHost::receivePacket(QByteArray &buffer) {
    while (m_socket.waitForReadyRead(m_timeout)) {
        // check datagram size
        qint64 size = m_socket.pendingDatagramSize();
        if (size >= 0) {
            // c64 only uses up to 512 bytes per datagram
            if (size > 512) {
                // discard datagram
                m_socket.readDatagram(0, 0);
            } else {
                // read datagram
                buffer.resize(size);
                QHostAddress addr;
                quint16 port;
                size = m_socket.readDatagram(buffer.data(), size, &addr, &port);
                if (size > 0) {
                    // datagram has valid size
                    if (buffer.size() != size)
                        buffer.resize(size);

                    // check address and port
                    if ((addr == m_addrPair.c64Addr) &&
                        (port == m_addrPair.c64Port)) {
                        emit receivedPacket(size);
                        return true;
                    }
                }
            }
        }
    }
    m_errorString =
        tr("No Datagram received in %1 msec").arg(QString::number(m_timeout));
    return false;
}

int NetHost::flushPackets(int timeout) {
    int num = 0;
    while (m_socket.waitForReadyRead(timeout)) {
        m_socket.readDatagram(0, 0);
        num++;
    }
    return num;
}
