#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef unsigned int uint;

struct PDU
{
    uint uiPDULen;       // 整个PDU的总长度，包括头部和数据部分
    uint uiMsgType;      // 消息类型，用于标识不同的操作（如登录、注册、文件传输等）
    char caData[64];     // 固定长度的数据区域，可用于存储用户名、文件名等固定长度信息
    uint uiMsgLen;       // 实际消息数据的长度，即caMsg部分的长度
    int caMsg[];         // 弹性数组，用于存储实际的消息数据
};

// 开辟一块PDU内存，大小为uiMsgLen + sizeof(PDU)
PDU* makePDU(uint uiMsgLen);

#endif // PROTOCOL_H
