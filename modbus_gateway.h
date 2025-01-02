#ifndef MODBUS_GATEWAY_H
#define MODBUS_GATEWAY_H
//----------------------------------------------------------------------------------------------------------------------
#include "chanlib_export.h"
#include "mxml.h"
#include "programthread.h"
#include "modbus_client.h"
//#include "protocol_adapter.h"
//----------------------------------------------------------------------------------------------------------------------
#define PACKET_SEND_TIMEOUT_MS 		3000
//----------------------------------------------------------------------------------------------------------------------
#define CHPLWRITELOG(format,...) {if(CHPL->logger) CHPL->logger->write(CHPL->DebugValue,format,##__VA_ARGS__);}
//----------------------------------------------------------------------------------------------------------------------
typedef struct _Session
{
	weak_ptr<BasicChannel> ch;
	uint32_t fd = 0;
	uint8_t	 deviceOnline = 0;
	timespec_t confirmStamp={0,0};
	timespec_t kASpan = {0,0};
	timespec_t kAStamp = {0,0};
	timespec_t kAStampR = {0,0};
	bool zipflag=0;
	uint32_t InSeq=0;
	uint32_t OutSeq=0;
	string chanaddr="";
}Session;
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
	virtual void process_channel(weak_ptr<BasicChannel> chan);
	//shared_ptr<ModbusGateway> passGW; // pointer to self
	shared_ptr<ChanPool> CHPL;
	vector<Session> sessionsActive;
	Session* currentSession=nullptr;
	mxml_node_t* config=nullptr;
	shared_ptr<ProtocolAdapter> adapter;
};
//----------------------------------------------------------------------------------------------------------------------
#endif/*MODBUS_GATEWAY_H*/