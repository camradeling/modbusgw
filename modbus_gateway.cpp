#include "mxmlconf.h"
#include "modbus_gateway.h"
#include "modbus_tcp_slave_adapter.h"
#include "modbus_uart_master_adapter.h"
#include "utils/timespec.h"
//----------------------------------------------------------------------------------------------------------------------
void ModbusGateway::init_module()
{
	CHPL = shared_ptr<ChanPool>(new ChanPool());
    CHPL->chp = CHPL;
	mxml_node_t* loggernode = mxmlFindElement(config, config, "Logger", NULL, NULL, MXML_DESCEND);
    if(loggernode)
    {
        const char* ext = mxmlGetText(loggernode,NULL);
        if(ext && string(ext) == "true") CHPL->logger = new ChannelLib::Logger;
    }
    shared_ptr<ChanPoolConfig> conf = shared_ptr<ChanPoolConfig>(mxml_parse_config(config));
	CHPL->init(conf.get());
	// currently I'll assume we have single ModbusTCP uplink and single UART downlink
	for(int i = 0; i < CHPL->allChan.size(); i++)
    {
		weak_ptr<BasicChannel> chan = CHPL->allChan.at(i);
		shared_ptr<BasicChannel> schan = chan.lock();
		CHPLWRITELOG("channel alias = %s\n", schan->alias.c_str());
		shared_ptr<ProtocolAdapter> adapter;
		if(schan->funcName == "ModbusTCPSlave") {
			CHPLWRITELOG("function = ModbusTCPSlave\n");
			adapter = uplink_adapter = shared_ptr<ProtocolAdapter>(new ModbusTCPSlaveAdapter(shared_from_this()));
			uplinkChannel = schan;
		}
		else if(schan->funcName == "ModbusUARTMaster") {
			CHPLWRITELOG("function = ModbusUARTMaster\n");
			adapter = downlink_adapter = shared_ptr<ProtocolAdapter>(new ModbusUARTMasterAdapter(shared_from_this()));
			downlinkChannel = schan;
		}
		if (!adapter)
			continue;
		add_pollable_handler(schan->inCmdQueue.fd(), EPOLLIN, &ModbusGateway::dispatch_event, this, chan, adapter);
        add_pollable_handler(schan->inQueue.fd(), EPOLLIN, &ModbusGateway::dispatch_event, this, chan, adapter);
    }
    
	ProgramThread::init_module();
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusGateway::thread_job()
{
	// TODO: add scheduled poll
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusGateway::dispatch_event(weak_ptr<BasicChannel> chan, shared_ptr<ProtocolAdapter> adapter)
{
	shared_ptr<BasicChannel> schan = chan.lock();
	if(!schan)
		return;
	std::unique_ptr<MessageBuffer> packet;
	while((packet = schan->inCmdQueue.pop()) && !stop)
	{
		enum MessageType packetType = packet->Type();
		if (packetType == CHAN_OPEN_PACKET)
		{
		    Session ses;
			ses.fd = packet->getfd();
			CHPLWRITELOG("[%d] connection established\n", ses.fd);
			ses.ch = schan;
			sessionsActive.push_back(ses);
			currentSession = &sessionsActive.at(sessionsActive.size()-1);
		}
		else if (packetType == CHAN_CLOSE_PACKET)
		{
			for (int j = 0; j < sessionsActive.size(); j++)
			{
				if (sessionsActive.at(j).fd == packet->getfd())
				{
					sessionsActive.erase(sessionsActive.begin() + j);
					break;
				}
			}
			currentSession = nullptr;
			CHPLWRITELOG("[%d] channel closed unexpectedly, cleanup and restart...\n", packet->getfd());
			// TODO: cleanup
		}
	}
    while((packet = schan->inQueue.pop()) && !stop)
	{
		enum MessageType packetType = packet->Type();
    	currentSession = nullptr;
        if (packetType == CHAN_DATA_PACKET)
        {
        	for (int j = 0; j < sessionsActive.size(); j++)
            {
            	if (sessionsActive.at(j).fd == packet->getfd())
                {
                	currentSession = &sessionsActive[j];
                    currentSession->InSeq = packet->seqnum;
                    adapter->process_packet(std::move(packet));
                    break;
                }
            }
        }
        else
        	CHPLWRITELOG("Unknown data packet type %d\n", packetType);
	}
	// process modbus requests
	timespec_t now;
	if (clock_gettime(CLOCK_REALTIME, &now)) {
		// TODO: error handling
	}
	std::vector<ModbusRequest>::iterator it;
	for (it = requests.begin(); it != requests.end(); ++it) {
		if (it->timestamp.tv_sec == 0) {
			// request not performed
			std::unique_ptr<MessageBuffer> packet(new MessageBuffer(0, ModbusMasterPacketConstructor::serialize_request(*it, MODBUS_TCP_PDU_TYPE), CHAN_DATA_PACKET));
			shared_ptr<BasicChannel> schan = downlinkChannel.lock();
			if (!schan) {
				CHPLWRITELOG("no downlink channel - how did that happen?\n");
			}
			schan->send_message_buffer(&schan->outQueue, std::move(packet), true);
		}
		else if (SLAVE_REPLY_TIMEOUT_MS < timespec_getdiff_ms(it->timestamp, now)) {
			CHPLWRITELOG("timeout happened - drop request\n");
			it = requests.erase(it);
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------
