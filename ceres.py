#!/usr/bin/python

import sys,os,time,random
import csv

import RPi.GPIO as GPIO
from mpd import MPDClient
from wakeonlan import wol

####
#CONFIG
####
#FILES
#Dir for the log files
LOG_DIR = '/home/pi/var/log/ceres/'
#Dir for cached and downloaded audio files
AUDIO_DIR = '/home/pi/var/audio/ceres/'
#Dir for home of the http-server
WEB_DIR = '/home/pi/srv/ceres/'

#NETWORK STUFF
#IP and MAC-Address for ENKIDU
ENKIDU_NET = {'IP': '192.168.0.101', 'MAC': 'E0.CB.4E.DA.68.7F'}
#Used Ports for ENKIDU
ENKIDU_PORTS = {'wakeonlan': 40000}
#IP and MAC-Address for MERCURIUS
MERCURIUS_NET = {'IP': '192.168.0.102', 'MAC': None}
#Used Ports for MERCURIUS
MERCURIUS_PORTS = {}

#HARDWARE STUFF
#Pins and their uses (BCM numbering)
PIN = {'door_switch': 26, 'door_opened_waiter': 16, 'alarm_switch': 16}

#RUNTIME STUFF
#Is the program in verbose mode?
VERBOSE_MODE = False

if VERBOSE_MODE:
	def vprint(*args):
		for arg in args:
			print arg,
		print
else:
	vprint = lambda *a: None

def setup_gpio():
	GPIO.setmode(GPIO.BCM)
	GPIO.setwarnings(False)
	return True

def is_mpd_playing():
	client = MPDClient()
	client.connect('localhost', 6600)
	client_state = client.status()['state']
	client.close()
	return client_state == 'play'

def ip_address_present(address):
	return os.system("fping -t50 -c1 " + address) == 0

class Log_Processer:
	def __init__(self, log_dir):
		self.log_dir = log_dir

	def append(self, preachmodule, message):
		vprint(preachmodule + ' logged: ' + message)

class Feedbacker:
	def event(self, event):
		if event == 'boot-up':
			self.boot_up()
		elif event == 'boot-down':
			self.boot_down()

	def speak(self, message):
		print message

	def audio(self, aim):
		print 'Playing ' + aim

	def boot_up(self):
		self.audio('boot sound')
		wol.send_magic_packet(ENKIDU_NET['MAC'], ip_address=ENKIDU_NET['IP'], port=ENKIDU_PORTS['wakeonlan'])
		self.speak('welcome master')

	def boot_down(self):
		self.speak('goodbye')
		if ip_address_present(ENKIDU_NET['IP']):
			os.system("sudo -u pi ssh morris@" + ENKIDU_NET['IP'] + " 'systemctl suspend'")

class Server:
	
	def __init__(self):
		print 'Starting Server'
	
class PreachModule:
	name = None
	logger = None
	feedback = None

class Door(PreachModule):
	##
	# LOGGER EVENTS
	# open	: Door opened
	# close	: Door closed (as in shut again)
	# left	: Master left the room
	# came	: Master arrived in the room
	##

	##
	# FEEDBACK EVENTS
	# boot-up	: booting up ceres fully
	# boot-down	: put ceres to suspend
	##
	name = 'Door'
	last_time_opened_bouncetime = 5

	def __init__(self, switch_pin, hold_pin, feedback, logger):
		self.switch_pin = switch_pin
		self.hold_pin = hold_pin
		self.feedback = feedback
		self.logger = logger
		self.is_at_home = True
		self.last_time_opened = time.time() - Door.last_time_opened_bouncetime
		setup_gpio()
		GPIO.setup(self.switch_pin, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)
		GPIO.setup(self.hold_pin, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)
		GPIO.remove_event_detect(self.switch_pin)
		GPIO.add_event_detect(self.switch_pin, GPIO.FALLING, callback = self.probe_opening, bouncetime = 50)

	def is_leaving_plausible(self):
		return not is_mpd_playing() and not ip_address_present(MERCURIUS_NET['IP'])

	def probe_opening(self, switch_pin):
		begin_opening = time.time()
		if (begin_opening - self.last_time_opened) <= Door.last_time_opened_bouncetime:
			return False
		time.sleep(0.3)
		for hold_counter in range(1,10):
			vprint(self.name + '_Hold: ' + '-' * hold_counter)
			if GPIO.input(self.switch_pin):
				return False
			time.sleep(0.05)
		self.logger.append(self.name, ['open', begin_opening])
		if self.is_at_home:
			self.probe_closing(self.hold_pin)
			if self.is_leaving_plausible():
				self.logger.append(self.name, ['left', begin_opening])
				self.is_at_home = False
				self.feedback.event('boot-down')
		else:
			self.feedback.event('boot-up')
			self.logger.append(self.name, ['came', begin_opening])
			self.is_at_home = True
			self.probe_closing(self.hold_pin)
		self.last_time_opened = begin_opening

	def probe_closing(self, hold_pin):
		GPIO.remove_event_detect(hold_pin)
		GPIO.wait_for_edge(hold_pin, GPIO.RISING)
		self.logger.append(self.name, ['close', time.time()])
		GPIO.remove_event_detect(hold_pin)

class Alarm(PreachModule):
	name = 'Alarm'

	def __init__(self, feedback, logger):
		self.feedback = feedback
		self.logger = logger

def main(argv):
	try:
		feedback = Feedbacker()
		logger = Log_Processer(LOG_DIR)
		door = Door(PIN['door_switch'], PIN['door_opened_waiter'], feedback, logger)
		
		while True:
			time.sleep(300)

	finally:
		GPIO.cleanup()

if __name__ == '__main__':
	main(sys.argv[1:])
