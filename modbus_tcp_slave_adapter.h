#ifndef MODBUS_TCP_SLAVE_ADAPTER_H
#define MODBUS_TCP_SLAVE_ADAPTER_H
//----------------------------------------------------------------------------------------------------------------------
#include "protocol_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
class ModbusTCPSlaveAdapter : public ProtocolAdapter
{
public:
    ModbusTCPSlaveAdapter(shared_ptr<ModbusGateway> SGW): ProtocolAdapter(SGW) {}
    virtual ~ModbusTCPSlaveAdapter(){}
    virtual void process_packet(uint8_t* data, int len);
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_TCP_SLAVE_ADAPTER*/
