import RPi.GPIO as GPIO
from mpd import MPDClient
from wakeonlan import wol
import os
import time

#The GPIO Pin Number for the door switch (BCM)
INPUT_PIN = 26
#File to hold the bool whether I'm at home
PRESENT_FILE = '/home/pi/tmp/door.io.present'
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
	if client_state == 'play':
		return True
	else:
		return False
	
def ip_address_present(address):
	is_up = os.system("ping -c 1 " + address)
	if is_up == 0:
		return True
	else:
		return False 
	
def start_welcome():
	wol.send_magic_packet(ENKIDU_MAC, ip_address=ENKIDU_IP, port=WOL_PORT)
	
def shutdown():
	if ip_address_present(ENKIDU_IP):
		os.system("sudo -u pi ssh morris@" + ENKIDU_IP + " 'systemctl suspend'")	
	
def add_callback(pin):
	GPIO.add_event_detect(pin, GPIO.FALLING, callback=test_open_door, bouncetime=50)

def test_open_door(pin):
	global time_stamp
	time_now = time.time()
	if (time_now - time_stamp) <= 5:
		return False
	time.sleep(0.3)
	for off_probe_counter in range(1,10):
		print '-' * off_probe_counter
		if GPIO.input(pin):
			return False
		time.sleep(0.05)
	time_opened = time.time()
	d_print('Door was opened')
	post_open_door()
	GPIO.remove_event_detect(pin)
	GPIO.wait_for_edge(pin, GPIO.RISING)
	d_print('and shut after {:.2f} sec'.format(time.time() - time_opened), False)
	time_stamp = time.time()
	GPIO.remove_event_detect(pin)
	add_callback(pin)
	
def post_open_door():
	open(PRESENT_FILE, 'a').close()
	with open(PRESENT_FILE, 'r+') as f_pres:
		b_pres = f_pres.readline().rstrip('\n')
		f_pres.seek(0)
		if b_pres == 'true' or not b_pres:
			if not is_mpd_playing() and not ip_address_present(MERCURIUS_IP):
				d_print('with Maurice leaving', False)
				f_pres.write('false')
				shutdown()
			else:
				f_pres.write('true')
		elif b_pres == 'false':
			d_print('with Maurice arriving', False)
			f_pres.write('true')
			start_welcome()
		else:
			d_print('ERROR: PRESENT_FILE has unreadable content')
		f_pres.truncate()

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(INPUT_PIN, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)

try:
	time_stamp = ((time.time()) - 10)
	add_callback(INPUT_PIN)
	
	while True:
		time.sleep(4)

finally:	
	GPIO.cleanup()
