/*
 * ESP8266-UART.cpp
 *
 *  Created on: 2017/08/23
 *      Author: ZBDMCM
 */

#include "ESP8266-UART.h"

Serial pc(USBTX, USBRX);
Serial uart(p0, p1);


void ESPUart::sendData(char sensData[]) {
	int response = 0;
	int index = 0;
	char data[256];

	for (int i = 0; i <= 1; i++) {
		switch (i) {
			case 0:
				sprintf(data,"10:\r");
				index = 10;
				break;
			case 1:
				sprintf(data,"20:%s\r",sensData);
				index = 20;
				break;
			default:
				index = 30;
				break;
		}

		pc.printf("senddata = \"%s\"\r\n", data);
		transmitData(data);
		response = resceiveData();

		if (response != index) {
			pc.printf("response Error !!\r\n");
			pc.printf("index = %d\r\n",index);
			pc.printf("response = %d\r\n",response);

		} else if (!checkResponse(response)){
			break;
		}
	}
}

void ESPUart::transmitData (char a[]) {
	uart.puts(a);
}

int ESPUart::resceiveData () {
	int tocnt = 0;
	int res = 0;

	//wait response from ESP8266
	while (uart.readable() != 1) {
		tocnt++;

		if (tocnt > 250) {
			pc.printf("\r\n");
			pc.printf("response error, please reset IDK and ESP8266 board\r\n");
			break;
		}else {
			pc.printf("/");
			wait(0.1);
		}
	}

	//Receive response from ESP8266;
	res = uart.getc();
	return res;

}

bool ESPUart::checkResponse(int c) {
	switch (c) {
		default:
			pc.printf("Error\r\n");
			return false;
			break;

		case 10:
			pc.printf("Response from WiFi OK\r\n");
			break;

		case 20:
			pc.printf("DATA send OK\r\n");
			break;
	}
	return true;
}
