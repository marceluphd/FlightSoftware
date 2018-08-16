/////////////////////////////////////////////////////////////////
////////////////// KICKSAT-2 FLIGHT SOFTWARE ////////////////////
/////////////////////////////////////////////////////////////////

/*
Last update on: 8-12-18
by: Ralen
*/

//////////////
// Includes //
//////////////
#include "IMUHandler.h"
#include <KickSatLog.h>
#include <RTCCounter.h>
#include <uplink.h>
#include <burn.h>
#include <KickSatConfig.h>
#include <BattHandler.h>
#include <kickSatGPS.h>

/////////////////
// Definitions //
/////////////////
#define HOLDTIME 900 // Time to hold after initial deployment, in seconds
#define BATTERYTHRESHOLD 2.055 // Battery must be above this threshold to exit standby mode
#define ANTENNA_WAIT 2 // How many beacons to wait in antenna wait, minutes
#define LISTENINGDURATION 5 // Defined in seconds
#define ARMINGDURATION 5 // Defined in seconds

///////////////////////////////////
// Declaration of global objects //
///////////////////////////////////
//IMUHandle IMU; // create IMU object
Counter watchdogTimer; // creates timer object
Counter beaconTimer; // creates timer object
Counter listenTimer; // creates timer object
BattHandle power;
GPSHandle kickSatGPS;

/////////////////////////////////
// Initialize global variables //
/////////////////////////////////
char buf[MAXCHARS]; // Create global variable for buffer from SD read function, this can be piped into radio.send()
//bool armingMode;

///////////////////////
// Declare functions //
///////////////////////
bool WDTFLAG = false; // Flag that allows toggling of the watchdog state

///////////
// SETUP //
///////////
void setup() {
  // Define pin modes
  pinMode(LED_BUILTIN, OUTPUT); // Defines builtin LED pin mode to output
  pinMode(WDT_WDI, OUTPUT); // Set watchdog pin mode to output

  // Begin timers
  watchdogTimer.init(1,watchdog); // timer delay, seconds

  // Initialize Serial
  SerialUSB.begin(115200); // Restart SerialUSB
  while(!SerialUSB); // Wait for SerialUSB USB port to open
  SerialUSB.println("SerialUSB Initialized");
  delay(5000); // Provides user with time to open SerialUSB Monitor or upload before sleep

  // Init objects
  if(configFile.init()) { // Initialize SD card and config file
    SerialUSB.println("Config File Initialized");
  } else {
    SerialUSB.println("Config File Not Initialized");
  }

  if(logfile.init()) { // Initialize log file
    SerialUSB.println("Log Card Initialized");
  } else {
    SerialUSB.println("Log Card Not Initialized");
  }

  if(IMU.begin()){ // Initialize IMU
    SerialUSB.println("IMU Intialized");
  } else {
    SerialUSB.println("IMU Could Not Be Intialized");
  }

  if(kickSatGPS.init()){ // Initialize IMU
    SerialUSB.println("IMU Intialized");
  } else {
    SerialUSB.println("IMU Could Not Be Intialized");
  }

  ///////////////
  // HOLD MODE //
  ///////////////
  // Goes into HOLD mode unpon initial deployment, flag is set to not enter this flag more than once
  if (!configFile.getHoldstatus()) { // If the satellite has just deployed and not been in HOLD mode yet (HOLD mode is the mandatory delay after deployment)
    SerialUSB.println("Entering HOLD mode");
    holdTimer.init(HOLDTIME,holdModeCallback); // timer delay, seconds
    timeout.start(HOLDTIME) // Sets up a backup timer to escape hold mode (while loop) (in case SD card is corrupted or something else unexpected)
    while(!configFile.getHoldstatus()) { // Hold here until we hit our hold timeout
      sleepTimer.sleep(); // Sleep
      if (timeout.triggered()) { // Checks time for timeout
        /* write hold status as completed (we only hold once) */
        break;
      }
    }

    /////////////////////
    // DEPLOY ANTENNAS //
    /////////////////////
    // Deploys antenna and updates status byte
    if (!configFile.getAB1status() || !configFile.getAB2status()) { // While either antenna burn wire is not burned
      SerialUSB.println("Begining Antenna Deployment Procedure");
      timeout.start(15); // Starts a timeout timer
      while(!configFile.getAB1status() || !configFile.getAB2status()) { // While either antenna burn wire is not burned
        if(!configFile.getAB1status() && batteryAboveThreshold()) { // Burn the first antenna wire if it hasn't already been done and the battery threshold is met
        burn.burnAntennaOne();
        configFile.setAB1Deployed();
        SerialUSB.println("Antenna One Burned");
      }
      if(!configFile.getAB2status() && batteryAboveThreshold()) { // Burn the second antenna wire if it hasn't already been done and the battery threshold is met
      burn.burnAntennaTwo();
      configFile.setAB2Deployed();
      SerialUSB.println("Antenna Two Burned");
    }
    if (timeout.triggered()) { // Checks time for timeout
      break;
    }
  }
}


}



// Begin beacon timer
beaconTimer.init(10); // timer delay, seconds

}
///////////////////////////////////////////////////////////////////

//////////
// LOOP //
//////////
void loop() {

  // Check if the beaconTimer has triggered and the battery is above the threshold voltage
  if (beaconTimer.check() && batteryAboveThreshold) {

    /////////////////////////////////////
    // Read all health and sensor data //
    /////////////////////////////////////
    createRandomData(); // Temporary solution TODO: Complete data collection for all sensors
    power.read(data.powerData); // Read IMU data TODO: This function doesn't exist but should
    kickSatGPS.read(data.gpsData); // Read IMU data TODO: This function doesn't exist but should
    //IMU.read(data.imuData); // Read IMU data


    //////////////////////////////////
    // Save all data to the SD card //
    //////////////////////////////////
    if(logfile.available()) {
      logfile.appendData();
    }

    //////////////////////////////////////////
    // Format health data and send to radio //
    //////////////////////////////////////////
    if(logfile.available()) {
      logfile.compileHealth(buf);
    }
    // radio.send(ax25(buf)); // Send health data through radio // TODO: This function doesn't exist yet but should

    //////////////////////////
    // Enter listening mode //
    //////////////////////////
    if (listenForUplink(buf, LISTENINGDURATION)) {
      // processUplink();
    }

    //////////////////////////
    // Enter listening mode //
    //////////////////////////
    if (armingMode) {
      if (listenForUplink(buf, ARMINGDURATION)) {
        processUplink(); // Process uplink while in arming mode
      }
    }

  }

  //////////////////////
  // Enter sleep mode //
  //////////////////////
  // sleepTimer.sleep(); // Go into sleep mode until next interrupt

}

void createRandomData() { // Temporary until we are     createRandomData(); // Temporary solution TODO: Complete data collection for all sensorsreading from each sensor
  data.status = random(0,10);
  for(uint8_t i = 0; i < 3; i++){
    data.powerData[i] = random(0,100)/13.87;
  }
  for(uint8_t i = 0; i < 2; i++){
    data.dateTime[i] = random(100000,999999);
  }
  for(uint8_t i = 0; i < 3; i++){
    data.gpsData[i] = random(0,200)/13.87;
  }
  for(uint8_t i = 0; i < 9; i++){
    data.imuData[i] = random(0,100)/9.123;
  }
}


bool batteryAboveThreshold() {
  return power.readBattVoltage() > BATTERYTHRESHOLD; // TODO: Read battery doesn't exist but should
  //return 3 > BATTERYTHRESHOLD;
}


void watchdog() { // Function that runs every time watchdog timer triggers
  if (WDTFLAG) {
    digitalWrite(LED_BUILTIN, HIGH); // Toggles builtin LED
    digitalWrite(WDT_WDI, HIGH); // Toggles watchdog timer
  } else {
    digitalWrite(LED_BUILTIN, LOW); // Toggles builtin LED
    digitalWrite(WDT_WDI, LOW); // Toggles watchdog timer
  }
  WDTFLAG = !WDTFLAG; // Toggles watchdog flag
}

void holdModeCallback() {
  //Increment time in config file on SD card
  if (/* timer value in config file >= HOLDTIME */) {
    /* write hold status as completed (we only hold once) */
  }
}