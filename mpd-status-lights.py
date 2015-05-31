import RPi.GPIO as GPIO
import mpd
import time

#WPI -> BCM Pin array
pins=[17,18,27,22,23,24,25,4]
play_pin = 5

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
client = mpd.MPDClient()

for pin in pins:
	GPIO.setup(pin, GPIO.OUT)
GPIO.setup(play_pin, GPIO.OUT)

try:
	while True:
		try:
			client.connect('localhost', 6600)
		except:
			client.ping()
			
		if client.status()['state'] in ['play', 'pause']:
			percent_elapsed = float(client.status()['elapsed'])/float(client.currentsong()['time'])
			for wPin in range(0, len(pins)):
				GPIO.output(pins[wPin], float(wPin)/float(len(pins)) <= percent_elapsed)
			GPIO.output(play_pin, client.status()['state'] == 'play')
		else:
			for pin in pins:
				GPIO.output(pin, False)
			GPIO.output(play_pin, False)
		time.sleep(3)
	
finally:
	GPIO.cleanup()
