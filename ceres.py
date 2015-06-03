import os,time,random
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
ENKIDU_NET = ['192.168.0.101','E0.CB.4E.DA.68.7F']
#Used Ports for ENKIDU
ENKIDU_PORTS = {'wakeonlan':40000}
#IP and MAC-Address for MERCURIUS
MERCURIUS_NET = ['192.168.0.102','']
#Used Ports for MERCURIUS
ERCURIUS_PORTS = {}

#HARDWARE STUFF
#Pins and their uses (BCM numbering)
PIN = {'door_switch':26, 'door_opened_waiter': 16, 'alarm_switch': 16}

class Log_Processer:

class Server:

class Door:
	
