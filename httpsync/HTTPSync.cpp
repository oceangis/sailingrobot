#include "HTTPSync.h"
//TODO - Oliver/Jordan: Sort this shit out
size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
    ((std::string*)stream)->append((char*)ptr, 0, size*count);
    return size*count;
}

HTTPSync::HTTPSync(DBHandler *db,int delay, bool removeLogs) : m_dbHandler(db) {
    m_running = true;
    m_removeLogs = removeLogs;
    curl = curl_easy_init();
    m_delay = delay;
}

HTTPSync::~HTTPSync() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void HTTPSync::run() {
    Logger::info("HTTPSync thread has started");

    setupHTTPSync();
    updateConfigs();

    pushWaypoints();
    pushConfigs();

    while(isRunning())
    {
        updateWaypoints();
        updateConfigs();
        pushDatalogs();
        std::this_thread::sleep_for(std::chrono::milliseconds(m_delay));
    }

    Logger::info("HTTPSync thread has exited");
}

void HTTPSync::setupHTTPSync() {

    m_pushOnlyLatestLogs = m_dbHandler->retrieveCellAsInt("httpsync_config", "1", "push_only_latest_logs");

    try {
        setShipID( m_dbHandler->retrieveCell("server", "1", "boat_id") );
        setShipPWD( m_dbHandler->retrieveCell("server", "1", "boat_pwd") );
        setServerURL( m_dbHandler->retrieveCell("server", "1", "srv_addr") );
    } catch (const char * error) {
        Logger::error("%s Failed to get server configuration Error: %s", __PRETTY_FUNCTION__, error);
        //Kill thread if setup fails
        std::terminate();
    }
    Logger::info("HTTPSync setup successfully");
}

void HTTPSync::pushDatalogs() {
    std::string response = "";
    if(performCURLCall(m_dbHandler->getLogs(m_pushOnlyLatestLogs), "pushAllLogs", response))
    {
         //remove logs after push
         //m_logger.info(response);
        if(m_removeLogs) {
            //m_dbHandler->removeLogs(response);
            m_dbHandler->clearLogs();
            //m_logger.info("response code: " + response);
        }

        Logger::info("Logs pushed to server");
    }
    else
    {
        Logger::warning("%s Could not push logs to server:", __PRETTY_FUNCTION__);
    }
}

void HTTPSync::pushWaypoints() {

	std::string waypointsData = m_dbHandler->getWaypoints();
	if (waypointsData.size() > 0)
	{
		std::string response;
		if(performCURLCall(waypointsData,"pushWaypoints", response))
		{
			Logger::info("Waypoints pushed to server");
		}
		else
		{
			Logger::warning("%s Failed to push waypoints to server", __PRETTY_FUNCTION__);
		}
	}
}

void HTTPSync::pushConfigs() {
    std::string response;
	if(performCURLCall(m_dbHandler->getConfigs(), "pushConfigs", response))
	{
		Logger::info("Configs pushed to server");
	}
	else
	{
		Logger::warning("%s Error: ", __PRETTY_FUNCTION__);
	}
}

void HTTPSync::setShipID(std::string ID) {
    shipID = ID;
}

void HTTPSync::setShipPWD(std::string PWD) {
    shipPWD = PWD;
}

void HTTPSync::setServerURL(std::string URL) {
    serverURL = URL;
}

void HTTPSync::setWaypointUpdateCallback(waypointUpdateCallback callBack){
    if (callBack != NULL){
        m_callBack = callBack;
    }
}

std::string HTTPSync::getData(std::string call) {
	std::string response;
    if(performCURLCall("",call, response))
    {
    	return response;
    }
    else
    {
    	return "";
    }
}


bool HTTPSync::checkIfNewConfig() {
    if (getData("checkIfNewConfigs") == "1")
        return true;

    return false;
}

bool HTTPSync::checkIfNewWaypoints(){
    if (getData("checkIfNewWaypoints") == "1")
   	    return true;

    return false;
}

void HTTPSync::updateConfigs() {
    if(checkIfNewConfig()) {

		std::string configs = getData("getAllConfigs");
		if (configs.size() > 0)
		{
			m_dbHandler->updateConfigs(configs);
			if (not m_dbHandler->updateTable("state", "configs_updated", "1", "1"))
			{
				Logger::error("%s Error updating state table",__PRETTY_FUNCTION__);
				return;
			}
			Logger::info("Configuration retrieved from remote server");
			pushConfigs();
		}
		else
		{
			Logger::error("%s Error: %s", __PRETTY_FUNCTION__);
		}
	}
}

void HTTPSync::updateWaypoints() {

    if(checkIfNewWaypoints()){

		std::string waypoints = getData("getWaypoints"); //Waypoints call implemented? //performCURLCall("", "getWaypoints")'
		if (waypoints.size() > 0)
		{
			if (m_dbHandler->updateWaypoints(waypoints))
			{
				Logger::info("Waypoints retrieved from remote server");
				if (m_callBack != NULL)
				{
					m_callBack();
				}
			}
		}
		else
		{
			Logger::warning("%s Could not updated waypoints",__PRETTY_FUNCTION__);
		}
    }
}


bool HTTPSync::isRunning() {
    bool running;
    m_mutex.lock();
    running = m_running;
    m_mutex.unlock();
    return running;
}

void HTTPSync::close() {
    m_mutex.lock();
    m_running = false;
    m_mutex.unlock();
}

bool HTTPSync::performCURLCall(std::string data, std::string call, std::string& response) {
    std::string serverCall = "";

    if(data != "")
        serverCall = "serv="+call + "&id="+shipID+"&pwd="+shipPWD+"&data="+data;
    else
        serverCall = "serv="+call + "&id="+shipID+"&pwd="+shipPWD;
        //serv=getAllConfigs&id=BOATID&pwd=BOATPW

    if(curl) {
    	//Send data through curl with POST
		curl_easy_setopt(curl, CURLOPT_URL, serverURL.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, serverCall.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			Logger::error("%s Error: %s response: %s", __PRETTY_FUNCTION__, curl_easy_strerror(res), response);
			return false;
		}
    }

    return true;
}
