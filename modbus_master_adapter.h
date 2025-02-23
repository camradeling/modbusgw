#ifndef MODBUS_MASTER_ADAPTER_H
#define MODBUS_MASTER_ADAPTER_H
//----------------------------------------------------------------------------------------------------------------------
#include "modbus.h"
#include "protocol_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
class ModbusMasterAdapter : public ProtocolAdapter
{
public:
    ModbusMasterAdapter(shared_ptr<ModbusGateway> SGW, modbus_pdu_type_e tp): ProtocolAdapter(SGW) { pdu_type = tp; }
    virtual ~ModbusMasterAdapter(){}
    virtual void process_packet(std::unique_ptr<MessageBuffer> packet);
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_MASTER_ADAPTER*/
