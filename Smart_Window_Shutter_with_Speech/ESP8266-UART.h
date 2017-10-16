#include <ticker.h>
#include "mbed.h"
#include "Shields.h"

class ESPUart {
	public:
		void sendData (char[]);
		void transmitData (char[]);
		int resceiveData ();
		bool checkResponse (int);
};
