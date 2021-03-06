
#pragma once

#include "SystemServices/Timer.h"
#include "SystemServices/Logger.h"
#include "MessageBus/MessageBus.h"
#include "DataBase/DBLoggerNode.h"
#include "../cxxtest/cxxtest/TestSuite.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <future>

#define DBLOGGERNODE_LOOP_TIME 1
#define DBLOGGERNODE_QUEUE_SIZE 1

class DBLoggerNodeSuite : public CxxTest::TestSuite {

    public:

    DBHandler* dbHandler;
    DBLoggerNode* dbLoggerNode;
    std::thread* thr;

    static MessageBus& msgBus(){
   	 	static MessageBus* mbus = new MessageBus();
    	return *mbus;
  	}

    static void runMessageLoop()
 	{
    	msgBus().run();
  	}

    void setUp() {
        if(dbLoggerNode == 0) {
            dbHandler = new DBHandler("../asr.db");
            dbLoggerNode = new DBLoggerNode(msgBus(), *dbHandler, DBLOGGERNODE_QUEUE_SIZE);
            thr = new std::thread(runMessageLoop);
        }
    }

    void tearDown() {
        dbLoggerNode->stop();
        msgBus().stop();
        thr->join();
        delete thr;
        delete dbLoggerNode;
        delete dbHandler;
    }

    void test_LoggingToDB() {

        if (Logger::init())
        {
		    Logger::info("Built on %s at %s", __DATE__, __TIME__);
		    Logger::info("Logger init\t\t[OK]");
	    }
	    else
        {
		    Logger::error("Logger init\t\t[FAILED]");
	    }

        if(dbHandler->initialise())
        {
		    Logger::info("Database init\t\t[OK]");
	    }
	    else
        {
		    Logger::error("Database init\t\t[FAILED]");
		    Logger::shutdown();
		    exit(1);
	    }

        dbLoggerNode->start();

        Timer timer;

        timer.sleepUntil(DBLOGGERNODE_LOOP_TIME + 0.1);

    }
};
