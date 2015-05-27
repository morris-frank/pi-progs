#include <wiringPi.h>

int main(void)
{
	wiringPiSetup();
	short pin;
	for(pin = 0; pin < 8; pin++)
		pinMode(pin, OUTPUT);
	pinMode(21, OUTPUT);
	pinMode(22, OUTPUT);
	for(;;)
	{
		for(pin = 0; pin < 23; pin++)
		{
			digitalWrite(pin, LOW);
			if(pin == 7)
				pin = 20;
			digitalWrite((pin+3)%23, HIGH);
			delay(50);
		}
	}
	return 0;
}