#ifndef __I2CCONTROLLER_H__
#define __I2CCONTROLLER_H__

#include <iostream>
#include "CV7/CV7.h"
#include "CV7/MockWindsensor.h"
#include "Compass/HMC6343.h"
#include "Compass/MockCompass.h"
#include "AnalogArduino/AR_UNO.h"
#include "AnalogArduino/MockAnalogArduino.h"
#include "dbhandler/DBHandler.h"
#include "thread/SystemState.h"

#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <string>
#include <mutex>

class I2CController {

	public:

		I2CController(SystemState *systemState, bool mockArduino, bool mockCompass, int headingBufferSize);
		~I2CController();

		void init();
		void run();

	private:
		SystemState* m_systemState;
		bool m_mockArduino;
		bool m_mockCompass;
		int m_headingBufferSize;
		Logger m_logger;
		std::mutex m_mutex;
		bool m_running;

		std::unique_ptr<Compass> m_compass;
		std::unique_ptr<AnalogArduino> m_arduino;

		bool isRunning();
		void initCompass(bool mockCompass,int headningBufferSize);
		void initArduino(bool mockArduino);
		void close();
};

#endif