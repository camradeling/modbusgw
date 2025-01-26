#include "modbus_uart_master_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
void ModbusUARTMasterAdapter::process_packet(std::unique_ptr<MessageBuffer> packet) 
{
    // testing echo
    fprintf(stderr, "ModbusUARTMasterAdapter::process_packet 1\n");
    shared_ptr<ModbusGateway> mbgw = MBGW.lock();
    if (!mbgw)
        return;
    if (!mbgw->currentSession)
        return;
    fprintf(stderr, "ModbusUARTMasterAdapter::process_packet 2\n");
}
//----------------------------------------------------------------------------------------------------------------------
