#include <Arduino.h>
#include <flight_timers.h>

//functions
void handle_config();
void init_timers();
void initial_mode();
void chirp_mode();
void chirp();
void check_deployment_condition();
void deployment();

//flags
int deploy_sprites = 0; //deployment flag, indicates when we are ready to deploy (3 month wait)
int antenna_deployed = 0; //holds antenna status
int high_chirp_freq = 0; //indicates whether we are using high or low frequency chirping
int chirp_signal = 0; //flag that goes high when its time to chirp

volatile int current_tick = 0; //global counter that increments every tick
volatile int time_before_next_chirp = 0; //decrements every second in WDT loop
volatile int time_before_next_sensor_data = 1200; //TODO: choose good initial value

bool high_frequency_mode = false;
int num_chirps_until_switch = 0;

#define uplink_num_chirps 120 //TODO: figure out actual values here and below
#define no_uplink_num_chirps 100
#define high_freq_num_chirps 300

//pin definitions
#define WDT_WDI 12
#define LED0 9


int main(void) {
	//once power is on, we begin our initialization sequence
	init_timers();
	handle_config();
	
	//check if the antenna is down, if so, go to initial_mode
	if(!antenna_deployed) {
		initial_mode(); //fix timers here
	}
	while(!deploy_sprites){
		chirp_mode();
	}
	//time to deploy
	deployment();
	while(true) { //go back to chirp mode after deploying sprites
		//could potentially just forever attempt to deploy, to ensure maximum likelihood of all burn wires working
		chirp_mode();
	}
}

void handle_config() {
	//TODO: read config files and set flags/variables to recover last status
}

void initial_mode() {
	//wait for 15 min, then extend antenna
	while(current_tick < 900) {}
	
	//extend antenna here
}

void init_timers() {
	//TODO: initialize the timers needed
	//Timers:
	//WDT - pings every second
	//init_WDT();
	//TODO: maybe other timers?
}

void chirp_mode() {

	while(time_before_next_chirp > 0) {
		//wait for timer to go off before chirping
		//TODO: uplink handling

		if (time_before_next_sensor_data <= 0) { //could be decremented in this loop or in WDT loop
			//gather_sensor_data();
		}

		//if (uplink detected) {
		//handle_uplink_data()
		high_frequency_mode = false;
		num_chirps_until_switch = uplink_num_chirps;
		//}
	}

	chirp();
	num_chirps_until_switch--;

	if (num_chirps_until_switch == 0) {
		//if we've chirped enough times, switch modes
		high_frequency_mode = !high_frequency_mode;
		if (high_frequency_mode) {
			//if we just entered HF mode, set the number of chirps to be done in this mode
			num_chirps_until_switch = high_freq_num_chirps;
		} else {
			//we must have just finished HF mode, with no uplink detected
			num_chirps_until_switch = no_uplink_num_chirps;
		}
	}

	//reset volatile counting integer
	if (high_frequency_mode) {
		time_before_next_chirp = 60; //1 minute until next chirp
		} else {
		time_before_next_chirp = 270; //4.5 minutes
	}
}

void chirp() {
	//TODO: collect self check data
	// check battery status
	// check solar panel status
	// check current time
	//TODO: write to self check data file
	//TODO: check for file corruption
}


void deployment() {
	//TODO: access deployment pre-compiled code and run
}


//Interrupts and Handlers

//WDT Loop
void TC3_Handler() {
	
	// Overflow interrupt triggered
	if ( TC3->COUNT16.INTFLAG.bit.OVF == 1 ) {
		//toggles WDT
		REG_PORT_OUTTGL0 = WDT_WDI;
		
		//increment/decrement any global volatiles here
		current_tick++;
		time_before_next_chirp--;
		time_before_next_sensor_data--;
		
		//clears the overflow interrupt flag
		REG_TC3_INTFLAG = TC_INTFLAG_OVF;
	}
	
	// Error interrupt triggered
	else if ( TC3->COUNT16.INTFLAG.bit.ERR == 1 ) {
		//possibly reset the micro if we ever get here
		
		//clears the error interrupt flag
		REG_TC3_INTFLAG = TC_INTFLAG_ERR;
	}
}