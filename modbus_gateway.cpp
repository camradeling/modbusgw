#include "mxmlconf.h"
#include "modbus_gateway.h"
#include "modbus_tcp_slave_adapter.h"
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
	if(CHPL->allChan.size() > 1)
	{
		CHPLWRITELOG("multiple channels in config, closing...\n");
		exit(-1);
	}
	for(int i = 0; i < CHPL->allChan.size(); i++)
    {
		weak_ptr<BasicChannel> chan = CHPL->allChan.at(i);
		shared_ptr<BasicChannel> schan = chan.lock();
		CHPLWRITELOG("channel alias = %s\n", schan->alias.c_str());
		if(schan->funcName == "ModbusTCPSlave") {
			CHPLWRITELOG("function = ModbusTCPSlave\n");
			adapter = shared_ptr<ProtocolAdapter>(new ModbusTCPSlaveAdapter(shared_from_this()));
		}
		add_pollable_handler(schan->inCmdQueue.fd(), EPOLLIN, &ModbusGateway::process_channel, this, chan);
        add_pollable_handler(schan->inQueue.fd(), EPOLLIN, &ModbusGateway::process_channel, this, chan);
    }
    
	//MBCL = shared_ptr<ModbusClient>(new ModbusClient(modbus_pdu_type));
    ProgramThread::init_module();
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusGateway::thread_job()
{
	/*vector<uint8_t> packdata;
	uint64_t diff=0;
	timespec_t curStamp = {0,0};
	shared_ptr<BasicChannel> schan;
	std::unique_ptr<MessageBuffer> buf;
	if(currentSession == nullptr)
		return;
	vector<uint8_t> pack = get_packet();
	if(pack.size())
		process_packet((uint8_t*)pack.data(), pack.size());
	if(packSent)
	{
		clock_gettime(CLOCK_MONOTONIC, &curStamp);
		diff = ((uint64_t)curStamp.tv_sec*1000 + (uint64_t)curStamp.tv_nsec/DECMILLION) - \
                    ((uint64_t)packSentStamp.tv_sec*1000 + (uint64_t)packSentStamp.tv_nsec/DECMILLION);
        if(diff > PACKET_SEND_TIMEOUT_MS)
        {
        	CHPLWRITELOG("timeout error, exiting...\n");
        	stop=1;
			exit(-1);
        }
	}*/
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusGateway::process_channel(weak_ptr<BasicChannel> chan)
{
	shared_ptr<BasicChannel> schan = chan.lock();
	if(!schan)
		return;
	std::unique_ptr<MessageBuffer> packet;
	while((packet = schan->inCmdQueue.pop()) && !stop)
	{
		enum MessageType packetType = packet->Type();
		if (packetType == CHAN_OPEN_PACKET && sessionsActive.size() == 0)
		{
		    Session ses;
			ses.fd = packet->getfd();
			CHPLWRITELOG("connection established\n");
			ses.ch = schan;
			ses.deviceOnline = 0;
			if(packet->getChanAddr() != "")
				ses.chanaddr = packet->getChanAddr();
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
			CHPLWRITELOG("command channel closed unexpectedly, cleanup and restart...\n");
			// TODO: cleanup
		}
	}
    while((packet = schan->inQueue.pop()) && !stop)
	{
		enum MessageType packetType = packet->Type();
    	currentSession = nullptr;
        if (packetType == CHAN_DATA_PACKET)
        {//для других зарезервируем это же значение
        	for (int j = 0; j < sessionsActive.size(); j++)
            {
                if (sessionsActive.at(j).fd == packet->getfd())
                {
                	currentSession = &sessionsActive[j];
                    currentSession->InSeq = packet->seqnum;
                    adapter->process_packet((uint8_t*)packet->Data(),packet->Length());
                    break;
                }
            }
        }
        else
        	CHPLWRITELOG("Unknown data packet type %d\n", packetType);
	}
}
//----------------------------------------------------------------------------------------------------------------------
