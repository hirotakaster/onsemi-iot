//============================================================================
// ----------------------------------------------------------------------------
// Copyright (c) 2016 Semiconductor Components Industries LLC
// (d/b/a "ON Semiconductor").  All rights reserved.
// This software and/or documentation is licensed by ON Semiconductor under
// limited terms and conditions.  The terms and conditions pertaining to the
// software and/or documentation are available at
// http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf ("ON Semiconductor Standard
// Terms and Conditions of Sale, Section 8 Software") and if applicable the
// software license agreement.  Do not use this software and/or documentation
// unless you have carefully read and you agree to the limited terms and
// conditions.  By using this software and/or documentation, you agree to the
// limited terms and conditions.
// ----------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// P.S:
//
//  1. This Application is to demonstrate Smart window shutter prototype.
//     The operations performed are as follows,
//
//     a. It simulates the positioning of window shutter using Stepper motor
//        rotation based on the intensity of light measured using ALS. As the
//        light increases, shutter closes more and vice versa.LCD displays the
//        status of motor rotation and ALS value on the LCD screen at a given time.
//
//        Ex: If the light intensity value(ALS reading) lies in between 1200
//            to 2000, then corresponding motor position will be +75 degree,
//            and whenever intensity value changes to above 2000, then motor
//            will rotate by 15 degree so that, the motor position will be at
//			  +90 degree.
//
//            For various scenarios, please refer the below table.
//             ___________________________________________________
//            | ALS value | Degree of rotation |  Shutter Status  |
//             ---------------------------------------------------
//            |    2000   |       90           | CompletelyClosed |
//             ---------------------------------------------------
//            |    1200   |       75           | PartialClosed    |
//             ---------------------------------------------------
//            |    400    |       0            | HalfOpen         |
//             ---------------------------------------------------
//            |    50     |       -75          | PartialOpen      |
//             ---------------------------------------------------
//            |    0      |       -90          | CompletelyOpen   |
//             ---------------------------------------------------
//
//     b. For Every change in the shutter position, current motor position,
//        status of shutter and light intensity(ALS) is uploaded to cloud.
//
//
//  2. The stack up diagram for this Complex application is as follows,
//
//     a. IDK power adaptor is needed to provide 12v/2A supply for Stepper motor
//        and baseboard.
//     b. Connect Power through USB/External supply 12v
//        <-> : PMOD Connection
//        ||  : Stack on top or bottom of IDK baseboard
//          ___________
//         |   WIFI    |
//         |___________|
//          __|_____|__
//         |  STEPPER  |->.............
//         |___________|->Stepper Motor2
//          __|_____|__     ___________
//         |   IDK     |<->|   ALS     |
//         |___________|   |___________|
//
//
//  3. For hardware and software capabilities/limitations kindly refer the
//     help page of respective modules in IDE. Other considerations with
//	   respect to given application are:
//
//     a. In case the board does not get reset upon pressing the reset button,
//        kindly disconnect the external power and USB cable,then connect back
//        the power and the USB cable.
//
//     b. Ambient light reading happens at every 5 seconds interval.
//
//
//  4. If the application has detailed logs, it is advisable to check the serial
//     console of the computer. To open the serial console use the following
//     settings:-
//
//     	Baud rate:    115200
//     	Data bits:    8
//     	Stop bits:    1
//     	Parity:       None
//     	Flow control: None
//
//    The above settings has been tested and verified on serial client putty.
//
// 5. If idle while loop is used in the project, then it is recommended to use
//	  a wait of 1 second to allow other threads/callbacks to run and get
//	  processed in time.
//-------------------------------------------------------------------------------


// Include Files
#include "header.h"


// Global variable declaration

// Configuration of window positions for each stop.
// First position to correspond to Window closed state &
// linearly to last being the fully open state.

struct windowConfiguration windowConfig[5] = {
		{ 2000, 90, "CompletelyClosed"},
		{ 1200, 75, "PartialClosed" },
		{ 400, 0, "HalfOpen"},
		{ 50, -75, "PartialOpen"},
		{ 0, -90, "CompletelyOpen"}
};

// Initial Motor position
int currMotorPosition;
char sendData[128];


/*******************************************************************************
 * API Definition
 ******************************************************************************/
/***************************************************************************//**
 * \fn         - main
 * \brief      - This function calls the respective functions for initialization of
 *               shields and controlling the shutter based on the ALS value.
 *               This application involves 3 shields :-
 *               1.ALS
 *               2.Stepper
 *
 *               a. The initialize_modules function initializes all these 2 shields.
 *               b. The als_based_shutter_operation function controls the stepper shield
 *                  based on the ALS value and displays the status on the LCD screen.
 * \returns    - int   - status (success/failure) based on the module initialization.
 ******************************************************************************/
int main() {
	//Function call to initialize the modules
	if (SWS_FAILURE == initializeModules()) {
		lcd.displayString("Initialize module failed");
		pc.printf("Initialization of modules failed\r\n");
		return SWS_FAILURE;
	} else {
		lcd.displayString("Initialize module successful");
		pc.printf("Initialization of modules successful\r\n");
		wait(1);
	}
	uart.baud(9600);

	//Function call to control the stepper based on ALS values and display the status on LCD.
	alsBasedShutterOperation();

    return SWS_SUCCESS;
}

/***************************************************************************//**
 * \fn         - initializeModules
 * \brief      - This function Checks for the initialization of the shields.
 * \returns    - int   Returns the status, based on the module initialization.
 ******************************************************************************/
int initializeModules() {
	//Initialize the lcd
	lcd.init();
	pc.printf("Smart Window Application\r\n");
	lcd.displayString("Smart Window Application");
    wait(1);

	//Checking for the ALS
	if (ALS_SUCCESS == als.init()) {
		pc.printf("\r\nAmbient Light Sensor Device matches\r\n");
		lcd.displayString("ALS initialization successful");
	} else {
		pc.printf("\r\nAmbient Light Sensor Device doesn't match\r\n");
		lcd.displayString("ALS initialization failed");
		return SWS_FAILURE;
	}

    // Enabling stepper motor
	if (STPR_RET_SUCCESS == stepper.enable()) {
		pc.printf("motor driver 2 : enabling the stepper motor 2 success ...\r\n");
		lcd.displayString("MOTOR2:\nEnable success");
	} else {
		pc.printf("motor driver 2 : enabling the stepper motor 2 failed !!!\r\n");
		lcd.displayString("MOTOR2:\nEnable Failed");
		lcd.displayString("ERROR !!!\nEXITING DEMO");
		return SWS_FAILURE;
	}

	wait(5);

	return SWS_SUCCESS;
}

/***************************************************************************//**
 * \fn         - alsBasedShutterOperation
 * \brief      - This function operates the shutter based on ALS values.
 ******************************************************************************/
void alsBasedShutterOperation() {

	// Destination motor position
	int dstMotorPosition;

	// Intensity bands for every 2 successive ALS reading
	int intensityBandFirst,intensityBandSecond;

	// Setting the initial position of the motor
	currMotorPosition = windowConfig[COMPLETE_CLOSE].motorPosition;

	while(1) {
		// Checking for any error in Stepper motor driver module
		if (stepper.checkStprErrorOut() == true) {
			lcd.displayString("Stepper Coil \n Error");
			pc.printf("STEPPER coil error !!!\r\n");
			return;
		}

		//Reading ALS data
		als.read(alsReadData);
		sprintf(cmd, "Light intensity %d", alsReadData);
		lcd.displayString(cmd);
		pc.printf("Light intensity is %d\r\n", alsReadData);

		// Getting the Intensity bands for the corresponding ALS read data.
		intensityBandFirst = getIntensityBand(alsReadData);
		wait(5);

		// Read again to check if light intensity is stable
		als.read(alsReadData);
		sprintf(cmd,"Light intensity %d",alsReadData);
		lcd.displayString(cmd);
		pc.printf("Light intensity is %d\r\n",alsReadData);

		intensityBandSecond = getIntensityBand(alsReadData);

		if (intensityBandFirst == intensityBandSecond) {
			dstMotorPosition = windowConfig[intensityBandFirst].motorPosition;
			pc.printf("IntensityBand: %d currPosition:%d dstPosition:%d \r\n", intensityBandFirst, currMotorPosition, dstMotorPosition );

			if (currMotorPosition != dstMotorPosition) {
				if (STPR_RET_SUCCESS == stepper.rotateAngle(dstMotorPosition-currMotorPosition)) {
					currMotorPosition = dstMotorPosition;
					sprintf(cmd,"Current Motor Position is %d",currMotorPosition);
					lcd.displayString(cmd);
					wait(1);

					//Sending data to cloud
					pc.printf("Data upload to cloud start!\r\n");
			    	lcd.displayString("Data upload Start!");
			    	sendToCloud(intensityBandFirst);

				} else {
                    pc.printf("Motor rotation failed\r\n");
                    lcd.displayString("Rotation failed\n");
                    stepper.disable();
                    // Enabling stepper motor
                    if (stepper.enable() != STPR_RET_SUCCESS) {
                        pc.printf("Stepper Enable failed\r\n");
                        lcd.displayString("Stepper Enable Failed\n");
                    }
                }
			}
		}
	}
}

/***************************************************************************//**
 * \fn         - getIntensityBand
 * \brief      - This function returns the index for intensity band corresponding to light intensity.
 * \param      - alsReading	- ALS read data
 * \return     - int		- index value for the matched intensity band range.
*********************************************************************************************/
int getIntensityBand(int alsReading) {
	int index = 0;
	for (index = 0; index < MAX_WINDOW_POSITIONS; index++) {
		if (alsReading > windowConfig[index].lightIntensityBand)
			if (alsReading > windowConfig[index].lightIntensityBand)
				break;
	}
	return index;
}

/***************************************************************************//**
 * \fn         - sendToCloud
 * \brief      - This function updates the light intensity and shutter status to cloud.
 * \param[in]  - cfgIndex - index value for the intensity band.
 ******************************************************************************/
void sendToCloud(int cfgIndex) {

	char data[64];
	sprintf(data, "{\"Light_Intensity\":%d,"
			"\"Motor_Angle\":%d}\r", alsReadData, currMotorPosition);

	esp.sendData(data);
}

/******************************************************************************/
