#include <wiringPi.h>
#include <stdio.h>

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
		for(pin = 0; pin < 8; pin++)
		{
			digitalWrite(pin, LOW);
			digitalWrite((pin+3)%8, HIGH);
			if(pin % 3 == 0){
				digitalWrite(21, HIGH);
				digitalWrite(22, LOW);
			}else{
				digitalWrite(21, LOW);
				digitalWrite(22, HIGH);
			}
			delay(50);
		}
	}
	return 0;
}