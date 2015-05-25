#include "xBeeSync.h"
#include <unistd.h>		//sleep

xBeeSync::xBeeSync(SystemState *systemState, DBHandler *db) :
	m_model(
		SystemStateModel(
			GPSModel("",0,0,0,0,0,0),
			WindsensorModel(0,0,0),
			CompassModel(0,0,0),
			0,
			0
		)
	),
	m_system_state(systemState),
	m_dbHandler(db),
	m_running(true)
{
	//	Hämtar ett heltal (1 eller 0) som visar om xbeen skall skicka och ta emot data.
	m_sending = m_dbHandler->retriveCellAsInt("configs", "1", "xb_send");
	m_receiving = m_dbHandler->retriveCellAsInt("configs", "1", "xb_recv");

	//	skapa ny xBee och kör xbee.init(baudRate (57600))
	m_xbee_fd = m_xBee.init(57600);
	std::cout << "*xBee initialized - receiving:" << m_receiving << " sending:" << m_sending << std::endl;
}

void xBeeSync::run()
{
	std::cout << "*xBeeSync thread started." << std::endl;
	while(isRunning()) 
	{
		m_system_state->getData(&m_model);

		if(m_sending)
		{
			std::string res_xml = m_XML_log.log_xml(
				m_model.gpsModel.timestamp,			// Timestamp
				m_model.windsensorModel.direction,	// winddir degrees
				m_model.windsensorModel.speed, 		// windspeed ms
				m_model.compassModel.heading, 		// Heading deg
				m_model.compassModel.pitch, 		// Pitch deg
				m_model.compassModel.roll, 			// Roll deg
				m_model.gpsModel.latitude, 			// gml:pos arg1, lat
				m_model.gpsModel.longitude, 		// gml:pos arg2, long
				m_model.gpsModel.heading, 			// course over ground(deg)
				m_model.gpsModel.speed, 			// speed over ground(ms)
				m_model.rudder,						// Rudderpos
				m_model.sail						// Sailpos
			);

			// Kan skicka loggen direkt med:
			m_xBee.sendXML(m_xbee_fd,res_xml);
		}
		
		if(m_receiving)
		{
			std::string res_xml = m_xBee.readOutput(m_xbee_fd);
 			
			//If return value equals -1, parsing failed...
 			int rudder_cmd = m_XML_log.parse_rudCMD(res_xml);
	 		int sail_cmd = m_XML_log.parse_saiCMD(res_xml);
			std::string timestamp = m_XML_log.parse_time(res_xml);
	 		
	 		if(timestamp.length() > 0) {
	 			std::cout << "Timestamp in xBeeSync::run = " << timestamp << std::endl;	
	 		}
	 		if(rudder_cmd != -1) {
	 			std::cout << "Rudder command in xBeeSync::run = " << rudder_cmd << std::endl;	
	 			m_model.rudder = rudder_cmd;
	 		}
	 		if(sail_cmd != -1) {
	 			std::cout << "Sail command in xBeeSync::run = " << sail_cmd << std::endl;	
	 			m_model.sail = sail_cmd;
	 		}
		}

		//make sure there are at least one second between each xml message
		sleep(1);
	}
	std::cout << "*xBeeSync thread exited." << std::endl;
}

void xBeeSync::close()
{
	m_mutex.lock();
	m_running = false;
	m_mutex.unlock();
}

bool xBeeSync::isRunning()
{
	bool running;
	m_mutex.lock();
	running = m_running;
	m_mutex.unlock();
	return running;
}