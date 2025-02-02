#include <stdint.h>
#include "modbus_master_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
void ModbusMasterAdapter::process_packet(std::unique_ptr<MessageBuffer> packet) 
{
    // received messaage from slave
    shared_ptr<ModbusGateway> mbgw = MBGW.lock();
    if (!mbgw)
        return;
    int offset = (MODBUS_TCP_PDU_TYPE == pdu_type) ? MODBUS_TCP_HEADER_OFFSET : 0;
    // simplify for now, we parse only modbus RTU
    // treating every packet as complete message
    if (packet->Length() < offset + 2)
        return; // TODO: error handling
    ModbusPDU reply = {
                .SlaveAddress = packet->Data()[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION + offset],
                .FunctionCode = packet->Data()[MODBUS_REQUEST_FUNCTION_CODE_POSITION + offset] 
            };
    // check if slaveID is in active list
    std::vector<ModbusPDU>::iterator it;
    for (it = mbgw->requests.begin(); it != mbgw->requests.end(); ++it) {
        if (it->SlaveAddress != reply.SlaveAddress)
            continue;
        switch (reply.FunctionCode)
        {
        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
            reply.cnt = packet->Data()[MODBUS_REPLY_BYTES_NUMBER_POSITION + offset] / 2;
            if (packet->Length() < 5 + reply.cnt*2) {
                fprintf(stderr, "unexpected end of packet\n");
                return; // TODO: error handling
            }
            for (int i = 0; i < reply.cnt; i++)
                reply.values.push_back(SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REPLY_REGISTER_DATA_START + offset + i*2]));
            break;
        case MODBUS_WRITE_SINGLE_REGISTER:
            // write single register reply is basically an echo of request
            reply.cnt = 1;
            reply.reg = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REPLY_REGISTER_ADDRESS_POSITION + offset]);
            reply.values.push_back(SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REQUEST_REGISTER_VALUE_POSITION + offset]));
            break;
        case MODBUS_LOOPBACK:
            reply.cnt = 0;
            break;
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            reply.reg = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REPLY_REGISTER_ADDRESS_POSITION + offset]);
            reply.cnt = SWAP16(*(uint16_t*)&packet->Data()[MODBUS_REPLY_REGISTER_NUMBER_POSITION + offset]);
            if (reply.cnt > MAX_REGS_TO_WRITE) {
                fprintf(stderr, "too many registers to report %d\n", reply.cnt);
                return; // TODO: error handling
            }
            break;
        }
        default:
            fprintf(stderr, "unknown modbus function code %d\n", reply.FunctionCode);
            return; // TODO: error handling
        }
        reply.transactionID = it->transactionID;
        fprintf(stderr, "created reply: ID: %04X, addr %d, func %d, register %04X, val/num %04X\n", reply.transactionID, reply.SlaveAddress, reply.FunctionCode, reply.reg, reply.cnt);
        // we need mutex here
        int fd = 0;
        shared_ptr<BasicChannel> schan = mbgw->uplinkChannel.lock();
        if (!schan) {
            fprintf(stderr, "no downlink channel - how did that happen?\n");
        }
        for (auto& session : mbgw->sessionsActive) {
            if (session.ch.lock() == schan) {
                fd = session.fd;
                break;
            }
        }
        std::unique_ptr<MessageBuffer> reply_packet(new MessageBuffer(fd, ModbusPacketConstructor::serialize_reply(reply, MODBUS_TCP_PDU_TYPE), CHAN_DATA_PACKET));
        schan->send_message_buffer(&schan->outQueue, std::move(reply_packet), true);
        it = mbgw->requests.erase(it);
        break;
    }
}
//----------------------------------------------------------------------------------------------------------------------
