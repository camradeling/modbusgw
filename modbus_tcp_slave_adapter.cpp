#include "modbus_tcp_slave_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
void ModbusTCPSlaveAdapter::process_packet(std::unique_ptr<MessageBuffer> packet) 
{
	// testing echo
	fprintf(stderr, "ModbusTCPSlaveAdapter::process_packet 1\n");
	shared_ptr<ModbusGateway> mbgw = MBGW.lock();
	if (!mbgw)
		return; // TODO: error handling
	if (!mbgw->currentSession)
		return; // TODO: error handling
	// let's assume we always receive full packet for now
	int offset = MODBUS_TCP_HEADER_OFFSET;
	if (packet->Length() < MODBUS_TCP_HEADER_OFFSET + 2)
		return; // TODO: error handling
	mbap_header_s* header = (mbap_header_s*)packet->Data();
	if (header->length < packet->Length() - MODBUS_TCP_HEADER_OFFSET)
		return; // TODO: error handling
	ModbusRequest req;
	req.SlaveAddress = packet->Data()[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION + MODBUS_TCP_HEADER_OFFSET];
	req.FunctionCode = packet->Data()[MODBUS_REQUEST_FUNCTION_CODE_POSITION + MODBUS_TCP_HEADER_OFFSET];
	req.reg = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_ADDRESS_POSITION + MODBUS_TCP_HEADER_OFFSET]);
	switch (req.FunctionCode)
	{
	case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
        req.cnt = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_NUMBER_POSITION + MODBUS_TCP_HEADER_OFFSET]);
        break;
    case MODBUS_WRITE_SINGLE_REGISTER:
        req.cnt = 1;
        req.values.push_back(SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_NUMBER_POSITION + MODBUS_TCP_HEADER_OFFSET]));
        break;
    case MODBUS_LOOPBACK:
        req.cnt = 0;
        break;
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
    {
        req.cnt = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_NUMBER_POSITION + MODBUS_TCP_HEADER_OFFSET]);
        if (req.cnt > MAX_REGS_TO_WRITE) {
        	fprintf(stderr, "too many registers to write %d\n", req.cnt);
			return;	// TODO: error handling
        }
        int numbytes = packet->Data()[MODBUS_REQUEST_BYTES_NUMBER_POSITION + MODBUS_TCP_HEADER_OFFSET];
        if (numbytes > req.cnt*2) {
        	fprintf(stderr, "number of bytes to write %d mismatch number of registers %d\n", numbytes, req.cnt);
			return;	// TODO: error handling
        }
        for (int i = 0; i < req.cnt; i++)
        	req.values.push_back(SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_DATA_START + MODBUS_TCP_HEADER_OFFSET + i*2]));
        break;
    }
	default:
		fprintf(stderr, "unknown modbus function code %d\n", req.FunctionCode);
		return;	// TODO: error handling
	}
	fprintf(stderr, "created request: addr %d, func %d, register %04X, val/num %04X\n", req.SlaveAddress, req.FunctionCode, req.reg, req.cnt);
	mbgw->requests.push_back(req);
}
//----------------------------------------------------------------------------------------------------------------------
