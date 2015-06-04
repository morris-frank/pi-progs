#!/usr/bin/python

#Standard Library
import sys
import getopt
import os
import time
from collections import deque
import wave
import alsaaudio
import random
import csv
import urllib

#External Modules
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


vprint = None


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
    return os.system("fping -q -t50 -c1 " + address) == 0


class URLopener(urllib.FancyURLopener):
    version = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.65 Safari/537.36'


class Log_Processer:
    def __init__(self, log_dir):
        self.log_dir = log_dir

    def append(self, preachmodule, message):
        vprint('[LOG] ' + preachmodule + ': ' + str(message))


class Feedbacker:
    url_opener = URLopener()
    audio_stack = deque()
    audio_stack_lock = False
    speech_stack = deque()
    speech_stack_lock = False

    def __init__(self, language='de', audio_card='default'):
        self.audio_card = audio_card
        self.language = language

    def event(self, event):
        if event == 'boot-up':
            self.boot_up()
        elif event == 'boot-down':
            self.boot_down()

    def speak(self, message):
        filename = 'SPEAK__' + str(message).replace(' ', '_')
        self.url_opener.retrieve('http://translate.google.com/translate_tts?tl=' + self.language + '&q=' + str(message),  AUDIO_DIR + filename + '.mp3')
        os.system('mpg123 -qw ' + AUDIO_DIR + filename + '.wav ' + AUDIO_DIR + filename + '.mp3')
        time.sleep(1)
        self.audio(filename + '.wav')

    def audio(self, aim, stack=False):
        if stack and self.audio_stack_lock:
            return self.audio_stack.append(aim)

        file_handler = wave.open(AUDIO_DIR + aim, 'rb')
        device_handler = alsaaudio.PCM(card=self.audio_card)
        device_handler.setchannels(file_handler.getnchannels())
        device_handler.setrate(file_handler.getframerate())

        if file_handler.getsampwidth() == 1:
            device_handler.setformat(alsaaudio.PCM_FORMAT_U8)
        elif file_handler.getsampwidth() == 2:
            device_handler.setformat(alsaaudio.PCM_FORMAT_S16_LE)
        elif file_handler.getsampwidth() == 3:
            device_handler.setformat(alsaaudio.PCM_FORMAT_S24_LE)
        elif file_handler.getsampwidth() == 4:
            device_handler.setformat(alsaaudio.PCM_FORMAT_S32_LE)
        else:
            raise ValueError('Unsupported format')

        device_handler.setperiodsize(320)
        data_step = file_handler.readframes(320)
        while data_step:
            device_handler.write(data_step)
            data_step = file_handler.readframes(320)
        if stack:
            if self.audio_stack:
                self.audio(self.audio_stack.popleft())
            else:
                self.audio_stack_lock = False

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
    # open  : Door opened
    # close : Door closed (as in shut again)
    # left  : Master left the room
    # came  : Master arrived in the room
    ##

    ##
    # FEEDBACK EVENTS
    # boot-up   : booting up ceres fully
    # boot-down : put ceres to suspend
    ##
    name = 'Door'
    last_time_opened_bouncetime = 7

    def __init__(self, switch_pin, hold_pin, feedback, logger):
        self.switch_pin = switch_pin
        self.hold_pin = hold_pin
        self.feedback = feedback
        self.logger = logger
        self.is_at_home = True
        self.last_time_opened = int(time.time()) - Door.last_time_opened_bouncetime
        setup_gpio()
        GPIO.setup(self.switch_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
        GPIO.setup(self.hold_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    def start(self):
        GPIO.remove_event_detect(self.switch_pin)
        GPIO.add_event_detect(self.switch_pin, GPIO.FALLING, callback=self.probe_opening, bouncetime=50)

    def stop(self):
        GPIO.remove_event_detect(self.switch_pin)

    def is_leaving_plausible(self):
        return not is_mpd_playing() and not ip_address_present(MERCURIUS_NET['IP'])

    def probe_opening(self, switch_pin):
        begin_opening = int(time.time())
        if (begin_opening - self.last_time_opened) <= self.last_time_opened_bouncetime:
            return False
        time.sleep(0.3)
        for hold_counter in range(1, 10):
            vprint('[INFO] ' + self.name + '_Hold: ' + '-' * hold_counter)
            if GPIO.input(self.switch_pin):
                return False
            time.sleep(0.05)
        self.logger.append(self.name, ['open', begin_opening])
        if self.is_at_home:
            self.probe_closing(self.hold_pin)
            if self.is_leaving_plausible():
                self.logger.append(self.name, ['left', begin_opening])
                self.is_at_home = False
                print "REALLY"
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
        self.logger.append(self.name, ['close', int(time.time())])
        GPIO.remove_event_detect(hold_pin)


class Alarm(PreachModule):
    name = 'Alarm'

    def __init__(self, feedback, logger):
        self.feedback = feedback
        self.logger = logger


def main(argv):
    global VERBOSE_MODE

    try:
        opts, args = getopt.getopt(argv, "hv")
    except getopt.GetoptError:
        print 'ceres.py [-h][-v]'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'ceres.py'
            sys.exit()
        elif opt == '-v':
            VERBOSE_MODE = True

    if VERBOSE_MODE:
        def _vprint(*args):
            for arg in args:
                print arg,
                print
    else:
        _vprint = lambda *a: None
    global vprint
    vprint = _vprint

    try:
        feedback = Feedbacker()
        logger = Log_Processer(LOG_DIR)
        door = Door(PIN['door_switch'], PIN['door_opened_waiter'], feedback, logger)
        door.start()
        while True:
            time.sleep(300)

    finally:
        GPIO.cleanup()


if __name__ == '__main__':
    main(sys.argv[1:])
