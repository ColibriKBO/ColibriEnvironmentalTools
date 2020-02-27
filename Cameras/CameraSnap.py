import urllib.request
import ftplib
import time
from ftpmod import *

def uploadFilesFTP(RemoteDir, server, username, password):
	ftp = ftplib.FTP(server)
	ftp.login(username, password)

	for i in range(len(iplist)):
		print('Uploading ' + imgname[i] + '.jpg')
		ftp.storbinary('STOR ' + imgname[i] + '.jpg', open('images/' + imgname[i] + '.jpg', 'rb'), 1024)

	ftp.quit()

iplist = ['24', '25', '26', '44', '45', '64', '65']
imgname = ['reddoor','redinside','redpov','greendoor','greeninside','bluedoor','blueinside']

while(1):
	try:
		for i in range(len(iplist)):
			image = urllib.request.urlopen('http://10.0.20.' + iplist[i] + '/webcapture.jpg?command=snap')
			outfile = open('images/' + imgname[i] + '.jpg', 'wb')
			outfile.write(image.read())
			outfile.close()
			print('Grabbed ' + imgname[i] + '.jpg from camera')

		uploadFilesFTP('blah', server, username, password)
	except:
		pass
	print('Waiting 60 seconds...')
	time.sleep(60)