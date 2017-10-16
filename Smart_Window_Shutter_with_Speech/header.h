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

#ifndef HEADER_H_
#define HEADER_H_

// Include Files
#include "mbed.h"
#include "Shields.h"
#include "ESP8266-UART.h"

// Macro declarations
#define NUM_DATA_ELEMENTS		3
#define MAX_BUFF_SIZE			128
#define ALS_INITIAL_VALUE		0

#define SWS_SUCCESS 			0
#define SWS_FAILURE				-1

// Enums declaration
enum windowPosition {
	COMPLETE_CLOSE = 0,
	PARTIAL_CLOSE,
	HALF_OPEN_OR_HALF_CLOSE,
	PARTIAL_OPEN,
	COMPLETE_OPEN,
	MAX_WINDOW_POSITIONS
};

// For configuration of window positions. 
// lightIntensityBand - Lower threshold for the intensity value
// motorPosition - Position of motor - degrees of angle
// windowState - Text string for display
struct windowConfiguration {
		int lightIntensityBand;
		int motorPosition;
		string windowState;
};

// Class instance declaration
Serial pc(p8,p9);
Serial uart(p0, p1);

NOA1305 als;
NHD_C0216CZ lcd;
AMIS30543D stepper(MOTOR2);
//WizFi250Interface eth;
ESPUart esp;


// Global variable declaration
int alsReadData = ALS_INITIAL_VALUE;
char cmd[MAX_BUFF_SIZE];
char buffer[MAX_BUFF_SIZE];

/*******************************************************************************
 * API DECLARATION
 ******************************************************************************/
/**
 * @fn      initializeModules
 * @brief
 * This function initializes the required modules.
 * @return
 * Status of function
 */

int initializeModules(void);

/**
 * @fn      alsBasedShutterOperation
 * @brief
 * This function handles the shutter movement based on ALS value.
 * @return
 * NONE
 */
void alsBasedShutterOperation(void);

/**
 * @fn      getIntensityBand
 * @brief
 * This function returns the index for intensity band corresponding to light intensity.
 * @param	alsReading
 * ALS read data
 * @return
 * Index value for the intensity band
 */
int getIntensityBand(int alsReading);

/**
 * @fn     sendToCloud
 * @brief
 * This function updates the light intensity and shutter status to cloud.
 * @param  cfgIndex
 * Index value of the intensity band
 * @return
 * NONE
 */
void sendToCloud(int cfgIndex);


#endif /* HEADER_H_ */
