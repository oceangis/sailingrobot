#ifndef __XBEESYNC_H__
#define __XBEESYNC_H__

#include "xBee/xBee.h"
#include "xmlparser/src/xml_log.h"
#include "thread/SystemState.h"
#include "models/SystemStateModel.h"
#include "dbhandler/DBHandler.h"
#include <mutex>

class xBeeSync
{
public:
	xBeeSync(SystemState *systemState, DBHandler *db);
	~xBeeSync() {};

	void run();
	void close();
	
private:
	XML_log m_XML_log;
	xBee m_xBee;
	int m_xbee_fd;
	SystemStateModel m_model;
	SystemState *m_system_state;
	DBHandler *m_dbHandler;

	std::mutex m_mutex;
	bool m_running;
	bool m_sending;
	bool m_receiving;

	bool isRunning();
};

#endif