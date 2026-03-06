#include "mytcpsocket.h"
#include <QDebug>
#include "protocol.h"
#include "operatedb.h"

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
    // qDebug() << pdu->uiMsgType << username << password;

    switch (pdu->uiMsgType)
    {
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_REQUEST):   // 注册请求
        {
            char username[32];
            char password[32];
            strncpy(username, pdu->caData, 32);
            strncpy(password, pdu->caData + 32, 32);
            PDU* respondPdu = makePDU(0);
            respondPdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND);

            if (OperateDB::getInstance().insertUserInfo(username, password))   // 插入成功
            {
                strcpy(respondPdu->caData, REGISTER_SUCCESSED);             // 向客户端发送成功的消息
            }
            else
            {
                strcpy(respondPdu->caData, REGISTER_FAILED);                // 向客户端发送失败的消息
            }
            this->write((char*)respondPdu, respondPdu->uiPDULen);
            free(respondPdu);
            respondPdu = nullptr;
            break;
        }
    }
    free(pdu);
    pdu = nullptr;
}
