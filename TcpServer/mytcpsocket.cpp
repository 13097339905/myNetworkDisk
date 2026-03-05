#include "mytcpsocket.h"
#include <QDebug>
#include "protocol.h"

MyTcpSocket::MyTcpSocket()
{
    connect(this, &MyTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));    // 先读取总长度4个字节出来，到uiPDULen
    uint uiMsglen = uiPDULen - sizeof(PDU);        // 得到消息长度
    PDU* pdu = makePDU(uiMsglen);                  // 根据消息长度构造出pdu
    // (char*)pdu + sizeof(uint):前四个字节之前已经读完了，所以要读到pud偏移之前读的位置上
    // uiPDULen - sizeof(uint):之前读的不用读了，所以要减去
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    qDebug() << pdu->uiMsgLen << " " << (char*)pdu->caMsg;
}
