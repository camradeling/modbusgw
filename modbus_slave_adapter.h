#ifndef MODBUS_SLAVE_ADAPTER_H
#define MODBUS_SLAVE_ADAPTER_H
//----------------------------------------------------------------------------------------------------------------------
#include "protocol_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
class ModbusSlaveAdapter : public ProtocolAdapter
{
public:
    ModbusSlaveAdapter(shared_ptr<ModbusGateway> SGW, modbus_pdu_type_e tp): ProtocolAdapter(SGW), pdu_type(tp) {}
    virtual ~ModbusSlaveAdapter(){}
    virtual void process_packet(std::unique_ptr<MessageBuffer> packet);
private:
    modbus_pdu_type_e pdu_type;
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_SLAVE_ADAPTER*/
