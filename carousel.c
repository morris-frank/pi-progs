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
			pin = pin == 7 ? 20 : pin;
			digitalWrite((pin+3)%8, HIGH);
			delay(50);
		}
	}
	return 0;
}