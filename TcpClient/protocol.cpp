#include "protocol.h"
#include <iostream>

PDU *makePDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU) + uiMsgLen;   // PDU大小
    PDU* pdu = (PDU*)malloc(uiPDULen);        // 开辟一块这么大的内存
    if (pdu == nullptr)
        exit(EXIT_FAILURE);
    pdu->uiPDULen = uiPDULen;    // 初始化PDU的总大小
    pdu->uiMsgLen = uiMsgLen;    // 初始化PDU的消息大小
    return pdu;
}
