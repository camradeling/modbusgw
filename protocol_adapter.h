#ifndef PROTOCOL_ADAPTER_H
#define PROTOCOL_ADAPTER_H
//----------------------------------------------------------------------------------------------------------------------
#include "modbus_gateway.h"
//----------------------------------------------------------------------------------------------------------------------
class ProtocolAdapter
{
public:
    weak_ptr<ModbusGateway> MBGW;
    ProtocolAdapter(shared_ptr<ModbusGateway> SGW):MBGW(SGW){}
    virtual ~ProtocolAdapter(){}
    virtual void process_packet(uint8_t* data, int len) = 0;
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*PROTOCOL_ADAPTER_H*/
