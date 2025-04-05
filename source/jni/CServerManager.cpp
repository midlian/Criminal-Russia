#include "CServerManager.h"

const char* g_szServerNames[MAX_SERVERS] = {
        "Criminal Russia Beta"
};

const CServerInstance::CServerInstanceEncrypted g_sEncryptedAddresses[MAX_SERVERS] = 
{
	CServerInstance::create("5.39.108.53", 1, 13, 1453, false) // FUSS: test
      //             CServerInstance::create("188.165.140.93", 1, 14, 1276, false) // rqce: buy
	//CServerInstance::create("141.95.190.138", 1, 14, 1125, false) // REVENGE: test
	//CServerInstance::create("141.95.234.17", 1, 13, 1517, false) // PP: buy
}; 