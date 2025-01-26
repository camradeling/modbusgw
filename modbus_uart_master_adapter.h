#ifndef MODBUS_UART_MASTER_ADAPTER_H
#define MODBUS_UART_MASTER_ADAPTER_H
//----------------------------------------------------------------------------------------------------------------------
#include "protocol_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
class ModbusUARTMasterAdapter : public ProtocolAdapter
{
public:
    ModbusUARTMasterAdapter(shared_ptr<ModbusGateway> SGW): ProtocolAdapter(SGW) {}
    virtual ~ModbusUARTMasterAdapter(){}
    virtual void process_packet(std::unique_ptr<MessageBuffer> packet);
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_UART_MASTER_ADAPTER*/
