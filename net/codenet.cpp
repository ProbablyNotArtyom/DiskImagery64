#include "codenet.h"

// ----- CodeNetTask -----

CodeNetTask::CodeNetTask(const QByteArray &packet) : m_packet(packet) {}

CodeNetTask::~CodeNetTask() {}

NetTask::Result CodeNetTask::run(NetHost &host) {
    quint8 seqNum = (char)m_packet[2];

    // write packet
    if (host.sendPacket(m_packet)) {
        // read result packet
        while (host.receivePacket(m_packet)) {
            // check if its the right result packet
            if ((m_packet[0] == (char)0xca) && (m_packet[1] == (char)0x1f) &&
                (m_packet[2] == (char)seqNum) && (m_packet[3] < (char)2)) {
                // packet ok
                bool ok = (m_packet[3] == (char)1);
                if (!ok) {
                    setStatusMessage(
                        QObject::tr("packet returned error status!"));
                    return ERROR;
                }
                return READY;
            }
        }
    }
    return RETRY;
}

// ----- CodeNetService -----

CodeNetService::CodeNetService() : NetService("CodeNet"), m_seqNum(0x41) {
    // set default c64 port
    m_addrPair.c64Port = 6462;
    m_addrPair.myPort = 6462;
}

CodeNetService::~CodeNetService() {}

void CodeNetService::sendData(quint16 addr, const QByteArray &data) {
    int size = data.size();
    int chunks = size / 128;
    int remainder = size % 128;

    int offset = 0;
    QByteArray block;
    block.resize(128);

    for (int i = 0; i < chunks; i++) {
        for (int j = 0; j < 128; j++)
            block[j] = data[offset++];
        sendBlock(addr, block);
        addr += 128;
    }
    if (remainder > 0) {
        block.resize(remainder);
        for (int j = 0; j < remainder; j++)
            block[j] = data[offset++];
        sendBlock(addr, block);
    }
    reportEvent(
        tr("sending data (%1 bytes)").arg(QString::number(data.size())));
}

void CodeNetService::sendBlock(quint16 addr, const QByteArray &data) {
    if (data.size() > 128)
        return;

    qint16 size = (quint16)data.size();
    m_packet.resize(8 + size);
    m_packet[4] = hi(addr);
    m_packet[5] = lo(addr);
    m_packet[6] = hi(size);
    m_packet[7] = lo(size);
    for (int i = 0; i < size; i++)
        m_packet[8 + i] = data[i];
    sendCommand(4);
}

void CodeNetService::fillRegion(quint16 addr, quint16 size, quint8 value) {
    m_packet.resize(9);
    m_packet[4] = hi(addr);
    m_packet[5] = lo(addr);
    m_packet[6] = hi(size);
    m_packet[7] = lo(size);
    m_packet[8] = value;
    sendCommand(5);
    reportEvent(tr("filling region (at 0x%1, %2 bytes with %3)")
                    .arg(QString::number(addr, 16))
                    .arg(QString::number(size))
                    .arg(QString::number(value)));
}

void CodeNetService::execJump(quint16 addr) {
    m_packet.resize(6);
    m_packet[4] = hi(addr);
    m_packet[5] = lo(addr);
    sendCommand(6);
    reportEvent(tr("execute jump to 0x%1").arg(QString::number(addr, 16)));
}

void CodeNetService::execRun() {
    m_packet.resize(4);
    sendCommand(7);
    reportEvent(tr("execute RUN command"));
}

void CodeNetService::sendCommand(quint8 cmd) {
    m_seqNum++;

    // assemble packet
    m_packet[0] = 0xca;
    m_packet[1] = 0x1f;
    m_packet[2] = m_seqNum;
    m_packet[3] = cmd;

    addTask(new CodeNetTask(m_packet));
}
