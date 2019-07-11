#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys

FILECALIBRATIONCONF='/data/etc/99-calibration.conf'
FILEBRIGHTNESS='/sys/class/backlight/nv_backlight/brightness'
FILEMAXBRIGHTNESS='/sys/class/backlight/nv_backlight/max_brightness'

def set_sound(args):
	print 'set sound', args
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
	backlight=brightness*100/maxbrightness
	f = open(FILEBRIGHTNESS, 'w')
	f.write(str(backlight))
	f.close()
	return 0

def get_sound():
	f=open(FILEMAXBRIGHTNESS, 'r')
	maxsound=int(f.read())
	f.close()
	f=open(FILEBRIGHTNESS, 'r')
	sound=int(f.read())
	f.close()
	print 'max sound', maxsound
	print 'sound', sound
	return sound*100/maxsound
	
def get_backlight():
	f=open(FILEMAXBRIGHTNESS, 'r')
	maxbrightness=f.read()
	f.close()
	f=open(FILEBRIGHTNESS, 'r')
	brightness=f.read()
	f.close()
	print 'max brightness', maxbrightness
	print 'brightness', brightness
	print int(brightness)*100/int(maxbrightness)
	return int(brightness)*100/int(maxbrightness)
	
def set_calibrator():
	os.system('xinput_calibrator --output-filename /data/etc/99-calibration.conf')
	return 0

print(len(sys.argv))
opt=''
arg=''
if len(sys.argv) >= 2:
	opt = sys.argv[1]
if len(sys.argv) >= 3:
	arg = sys.argv[2]

print('opt is ',opt)
print('arg is ',arg)

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
	
	
 
