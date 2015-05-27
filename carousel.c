#include <wiringPi.h>

int main(void)
{
	wiringPiSetup();
	short pin;
		for(pin = 0; pin < 8; pin++)
			pinMode(pin, OUTPUT);
	for(;;)
	{
		for(pin = 0; pin < 8; pin++)
		{
			digitalWrite(pin, LOW);
			digitalWrite((pin+2)%8, HIGH);
			delay(200);
			/*pin = pin == 7 ? 20 : pin;*/
		}
	}
	return 0;
}