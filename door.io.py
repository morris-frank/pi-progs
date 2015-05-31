import RPi.GPIO as GPIO
from mpd import MPDClient
from wakeonlan import wol
import os
import time

#The GPIO Pin Number for the door switch (BCM)
INPUT_PIN = 26
#File to hold the bool whether I'm at home
#PRESENT_FILE = '/home/pi/tmp/door.io.present'
#Global variable to hold whether I'm present
IS_PRESENT = True
#Time in sec, I was away or at home
LAST_CHANGE = time.time()
#LOG - File
LOG_FILE = '/home/pi/tmp/door.io.log'
#BOUND local IP address of my laptop
MERCURIUS_IP = '192.168.0.102'
#MAC address of enkidu
ENKIDU_MAC = 'E0.CB.4E.DA.68.7F'
#BOUND local IP address of enkidu
ENKIDU_IP = '192.168.0.101'
#Forwarded port to wol enkidu
WOL_PORT = 40000

def d_print(message, print_time=True):
	if print_time:
		message = time.ctime() + "\n" + message
	print(message)
	with open(LOG_FILE, 'a') as f_log:
		print >> f_log, message


def is_mpd_playing():
	client = MPDClient()
	client.connect('localhost', 6600)
	client_state = client.status()['state']
	client.close()
	return client_state == 'play'

def ip_address_present(address):
	return os.system("ping -c 1 " + address) == 0

def start_welcome():
	wol.send_magic_packet(ENKIDU_MAC, ip_address=ENKIDU_IP, port=WOL_PORT)

def shutdown():
	if ip_address_present(ENKIDU_IP):
		os.system("sudo -u pi ssh morris@" + ENKIDU_IP + " 'systemctl suspend'")

def add_callback(pin):
	GPIO.add_event_detect(pin, GPIO.FALLING, callback=test_open_door, bouncetime=50)
	
def shut_door_end(pin, opening_starttime):
	GPIO.remove_event_detect(pin)
	GPIO.wait_for_edge(pin, GPIO.RISING)
	d_print('and shut after {:.2f} sec'.format(time.time() - opening_starttime), False)

def test_open_door(pin):
	global last_time_opened, IS_PRESENT
	opening_starttime = time.time()
	if (opening_starttime - last_time_opened) <= 5:
		return False
	time.sleep(0.3)
	for off_probe_counter in range(1,10):
		print '-' * off_probe_counter
		if GPIO.input(pin):
			return False
		time.sleep(0.05)
	d_print('Door was opened')
	if IS_PRESENT:
		shut_door_end(pin, opening_starttime)
		if not is_mpd_playing() and not ip_address_present(MERCURIUS_IP):
			d_print('with Master leaving', False)
			d_print('after he was ' + str(opening_starttime - LAST_CHANGE) + 'sec at home', False)
			IS_PRESENT = False
			LAST_CHANGE = opening_starttime
			shutdown()
	else:
		start_welcome()
		d_print('with Master arriving', False)
		d_print('after he was ' + str(opening_starttime - LAST_CHANGE) + 'sec away from home', False)
		IS_PRESENT = True
		LAST_CHANGE = opening_starttime
		shut_door_end(pin, opening_starttime)
	last_time_opened = opening_starttime
	GPIO.remove_event_detect(pin)
	add_callback(pin)
	return True

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(INPUT_PIN, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)

try:
	last_time_opened = ((time.time()) - 10)
	add_callback(INPUT_PIN)

	while True:
		time.sleep(10)

finally:
	GPIO.cleanup()
