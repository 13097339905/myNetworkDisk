#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef unsigned int uint;

struct PDU
{
    uint uiPDULen;       // 总的协议数据单元大小
    uint uiMsgType;      // 消息类型
    char caData[64];     //
    uint uiMsgLen;       // 实际消息长度
    int caMsg[];         // 实际消息
};

// 开辟一块PDU内存，大小为uiMsgLen + sizeof(PDU)
PDU* makePDU(uint uiMsgLen);

#endif // PROTOCOL_H
