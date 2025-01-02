#include "modbus_tcp_slave_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
void ModbusTCPSlaveAdapter::process_packet(uint8_t* data, int len) 
{
	// testing echo
	fprintf(stderr, "process_packet\n");
	shared_ptr<ModbusGateway> mbgw = MBGW.lock();
	if (!mbgw)
		return;
	if (!mbgw->currentSession)
		return;
	fprintf(stderr, "process_packet loopback\n");
	std::unique_ptr<MessageBuffer> packet(new MessageBuffer(mbgw->currentSession->fd, len, CHAN_DATA_PACKET));
	memcpy(packet->Data(), data, len);
	mbgw->currentSession->ch.lock()->outQueue.push(std::move(packet), true);
}
//----------------------------------------------------------------------------------------------------------------------
