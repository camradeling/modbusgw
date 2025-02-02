#ifndef MODBUS_GATEWAY_H
#define MODBUS_GATEWAY_H
//----------------------------------------------------------------------------------------------------------------------
#include "chanlib_export.h"
#include "mxml.h"
#include "programthread.h"
#include "modbus_client.h"
//----------------------------------------------------------------------------------------------------------------------
#define SLAVE_REPLY_TIMEOUT_MS 		3000
//----------------------------------------------------------------------------------------------------------------------
#define CHPLWRITELOG(format,...) {if(CHPL->logger) CHPL->logger->write(CHPL->DebugValue,format,##__VA_ARGS__);}
//----------------------------------------------------------------------------------------------------------------------
typedef struct _Session
{
	weak_ptr<BasicChannel> ch;
	uint32_t fd = 0;
	uint32_t InSeq=0;
	uint32_t OutSeq=0;
} Session;
//----------------------------------------------------------------------------------------------------------------------
class ProtocolAdapter;
//----------------------------------------------------------------------------------------------------------------------
class ModbusGateway : public ProgramThread, public std::enable_shared_from_this<ModbusGateway>
{
public:
	ModbusGateway(mxml_node_t* cnf=nullptr):config(cnf){}
	virtual ~ModbusGateway(){}
	virtual void init_module();
	virtual void thread_job();
	virtual void process_requests();
	virtual void dispatch_event(weak_ptr<BasicChannel> chan, shared_ptr<ProtocolAdapter> adapter);
	shared_ptr<ChanPool> CHPL;
	std::vector<Session> sessionsActive;
	Session* currentSession=nullptr;
	mxml_node_t* config=nullptr;
	weak_ptr<BasicChannel> uplinkChannel;
	weak_ptr<BasicChannel> downlinkChannel;
	shared_ptr<ProtocolAdapter> uplink_adapter;
	shared_ptr<ProtocolAdapter> downlink_adapter;
	std::vector<ModbusPDU> requests;
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_GATEWAY_H*/