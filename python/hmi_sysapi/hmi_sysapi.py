#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys
import os

FILECALIBRATIONCONF='/data/etc/99-calibration.conf'
FILEBRIGHTNESS='/sys/class/backlight/pwm-backlight.0/brightness'
FILEMAXBRIGHTNESS='/sys/class/backlight/pwm-backlight.0/max_brightness'
SOUNDCHANNEL='Headphone'

def set_sound(args):
	sound=int(args)
	if sound < 1:
		sound = 0
	if sound > 100:
		sound = 100
	command='amixer sset ' + SOUNDCHANNEL + ' ' + str(sound) + '%'
	#print command
	os.system(command)
	return 0
	
def set_backlight(args):
	f=open(FILEMAXBRIGHTNESS, 'r')
	maxbrightness=int(f.read())
	f.close()
	brightness=int(args)
	print 'max brightness', maxbrightness
	print 'set brightness', brightness
	if brightness < 1:
		brightness = 1
	if brightness > 100:
		brightness = 100
	backlight=brightness*maxbrightness/100
	f = open(FILEBRIGHTNESS, 'w')
	f.write(str(backlight))
	f.close()
	return 0

def get_sound():
	command="amixer sget " + SOUNDCHANNEL + " |tail -n 1 |grep %|cut -d'%' -f1|cut -d'[' -f2"
	#print command
	p=os.popen(command) 
	sound=p.read()
	#print sound
	p.close()
	return int(sound)
	
def get_backlight():
	f=open(FILEMAXBRIGHTNESS, 'r')
	maxbrightness=f.read()
	f.close()
	f=open(FILEBRIGHTNESS, 'r')
	brightness=f.read()
	f.close()
	#print 'max brightness', maxbrightness
	#print 'brightness', brightness
	#print int(brightness)*100/int(maxbrightness)
	return int(brightness)*100/int(maxbrightness)
	
def set_calibrator():
	command='xinput_calibrator --output-filename ' + FILECALIBRATIONCONF
	os.system(command)
	return 0

#print(len(sys.argv))
opt=''
arg=''
if len(sys.argv) >= 2:
	opt = sys.argv[1]
if len(sys.argv) >= 3:
	arg = sys.argv[2]

#print('opt is ',opt)
#print('arg is ',arg)

if opt=='setsound':
	sys.exit(set_sound(arg))
elif opt =='getsound':
	sys.exit(get_sound())
elif opt=='setbacklight':
	sys.exit(set_backlight(arg))
elif opt=='getbacklight':
	sys.exit(get_backlight())
elif opt=='setcalibrator':
	sys.exit(set_calibrator())
	
	
 
