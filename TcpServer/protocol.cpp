#include "protocol.h"
#include <iostream>
#include <cstring>

// 分配内存给pdu并且初始化pdu的总长度和消息长度
PDU *makePDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU) + uiMsgLen;   // PDU大小
    PDU* pdu = (PDU*)malloc(uiPDULen);        // 开辟一块这么大的内存
    if (pdu == nullptr)          // 开辟内存失败退出
        exit(EXIT_FAILURE);
    memset(pdu, '\0', uiPDULen);    // 初始化所有字段为0，避免脏数据
    pdu->uiPDULen = uiPDULen;    // 初始化PDU的总大小
    pdu->uiMsgLen = uiMsgLen;    // 初始化PDU的消息大小
    return pdu;
}
