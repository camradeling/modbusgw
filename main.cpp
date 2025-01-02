#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include <iterator>
//----------------------------------------------------------------------------------------------------------------------
#include "modbus_gateway.h"
#include "chanlib_export.h"
//----------------------------------------------------------------------------------------------------------------------
const char* defaultConf = (char*)"./config.xml";
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    string configfile;
    int option_index = 0;
    int c;
    static struct option long_options[] = {
            {"config",  required_argument,NULL,0},
            {NULL,    0,                 NULL,  0 }
    };
    while((c = getopt_long(argc, argv, "c:",long_options,&option_index)) != -1)
    {
        switch(c)
        {
            case 0:
                if(option_index == 0)
                {
                    configfile = string(optarg);
                }
                break;
            case 'c':
                configfile = string(optarg);
                break;
            case '?':
                fprintf(stderr, "unknown option: %c\n", option_index);
                break;
        }
    }
	if(configfile == "")
    {
        configfile = string(defaultConf);
    }
    fprintf(stderr, "configuration file = %s\n",configfile.c_str());
    bool confok = 1;
    FILE* fpconf=nullptr;
    mxml_node_t* tree = nullptr;
    if ((fpconf = fopen(configfile.c_str(), "r")) != NULL)        // открыть файл с конфигурацией в формате XML
    {
        tree = mxmlLoadFile (nullptr, fpconf, MXML_NO_CALLBACK);       // считать конфигурацию
        fclose(fpconf);
        if (tree == nullptr)
        {
            fprintf(stderr, "config file invalid\n");
            confok = 0;
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "cant open config file\n");
        confok = 0;
        return -1;
    }
    fprintf(stderr, "config file valid\n");
    shared_ptr<ModbusGateway> mgw = shared_ptr<ModbusGateway>(new ModbusGateway(tree));
    mgw->init_module();
    fprintf(stderr, "loop...\n");
    while(!mgw->stop)
        sleep(1);
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------