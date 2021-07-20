import serial
import time
import csv

time.sleep(2)

ser = serial.Serial('COM4',115200)

data= []

while True:
	try:
		b = ser.readline()
		b2 = b.decode()
		b3 = b2.rstrip()
		print(b3)
		with open('lightning.csv', 'a') as f:
			writer = csv.writer(f, delimiter=' ')
			writer.writerow([time.time(),b3])
	except:
		print("Interrupt")
		break

ser.close()