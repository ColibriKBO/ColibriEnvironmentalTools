import sys, time, os
import ftplib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.path as mpath
import matplotlib.lines as mlines
import matplotlib.patches as mpatches
import matplotlib.gridspec as gridspec
import astropy.units as u 
from ftpmod import *
from socket import *
from matplotlib.collections import PatchCollection
from matplotlib import font_manager as fm
from pylab import *
from astropy.time import Time
from astropy.coordinates import SkyCoord, EarthLocation, AltAz, get_sun, get_moon
from astropy.utils import data
from datetime import datetime as dt
from datetime import timedelta
from scipy import interpolate
from time import sleep


def uploadFileFTP(sourceFile1, server, username, password):
	print('Uploading ' + sourceFile1)
	ftp = ftplib.FTP(server)
	ftp.login(username, password)
	ftp.storbinary('STOR ' + sourceFile1, open(sourceFile1, 'rb'), 1024)
	ftp.quit()
	print('Done uploading...')

def label(xy, text):
	y = xy[1] - 0.15  # shift y-value for label so that it's below the artist
	plt.text(xy[0], y, text, ha="center", family='sans-serif', size=14)

def sliceCBar(maxspeed, cbarname):
	# if speed > maxspeed:
	# 	colorindex = maxspeed - 1
	# else:
	# 	colorindex = speed - 1

	cmap = cm.get_cmap(cbarname, maxspeed)    # PiYG
	cmaparr = []

	for i in range(cmap.N):
		rgb = cmap(i)[:3] # will return rgba, we take only first 3 so we get rgb
		cmaparr.append(matplotlib.colors.rgb2hex(rgb))

	return cmaparr

def rot_text(ang): 
	rotation = np.degrees(np.radians(ang) * np.pi / np.pi - np.radians(90))
	return rotation

def spdang(spd):
	maxspeed = 30
	angle = 180.0 - (spd/maxspeed*180.0)
	return angle

def openCloudLog(logname):
	print('Making daily plot...')
	data = pd.read_csv(logname)
	data.columns = ['timestamp','SkyT','GroundT']

	t = data['timestamp']
	y = data['SkyT']-data['GroundT']
	temp = data['GroundT']

	# Convert timestamp to hours and put in a column in data
	data['timeh'] = 99

	for i in range(len(data.index)):
		t2 = int(dt.fromtimestamp(data['timestamp'][i]).strftime('%H')) + float(dt.fromtimestamp(data['timestamp'][i]).strftime('%M'))/60 + float(dt.fromtimestamp(data['timestamp'][i]).strftime('%S'))/3600
		data.loc[i,'timeh'] = t2

	x = data['timeh']

	# Arrange the data into blocks and calculate the mean of each block
	# blocksize = 600
	samplerate = 6
	f = interpolate.interp1d(t,y)
	xnew = np.arange(min(data['timestamp']),max(data['timestamp']),samplerate)
	ynew = f(xnew)
	# interval = 1
	n = int(len(ynew))

	a = ynew[0:(n-1)].reshape(1,1,n-1)
	block = np.mean(a, axis=1)

	return block, x, y, temp

def main():

	data.clear_download_cache()

	wx_address= (b'172.16.61.10', 17770)
	cld_address = (b'10.0.20.10', 8888)

	cld_socket = socket(AF_INET, SOCK_DGRAM)
	cld_socket.settimeout(2)

	while(1):
		req_data = b'READ\n'

		wx_socket = socket(AF_INET, SOCK_STREAM)
		wx_socket.settimeout(10)
		wx_socket.connect(wx_address)

		try:
			print('Trying...')
			wx_socket.sendall(req_data)
			rec_data = wx_socket.recv(1024)

			now = dt.now()
			msg = str(rec_data, 'utf-8')
		
			msg = msg.split('\n')

			baro = int(float(msg[4].split()[1]))
			t_in = float(msg[5].split()[1])
			t_out = float(msg[6].split()[1])
			t_obs = int(float(msg[7].split()[1]))
			h_in = int(float(msg[9].split()[1]))
			h_out = int(float(msg[10].split()[1]))
			v_wnd = int(float(msg[11].split()[1]))
			d_wnd = int(float(msg[13].split()[1]))+180
			v_wnd_av = int(float(msg[12].split()[1]))
			dark = int(float(msg[14].split()[1]))
			solar = int(float(msg[17].split()[1]))
			rain = int(float(msg[18].split()[1]))
			dew = int(float(msg[19].split()[1]))

			datenow = now.strftime("%Y-%m-%d")
			timenow = now.strftime("%H:%M:%S")
			wnddir = np.radians(d_wnd)

			elginfield = EarthLocation(lat=43.192*u.deg, lon=-81.318*u.deg, height=586*u.m)
			moonaz = "{:.1f}".format((get_moon(Time.now()).transform_to(AltAz(obstime=Time.now(), location=elginfield)).az*u.deg).value)
			moonalt = "{:.1f}".format((get_moon(Time.now()).transform_to(AltAz(obstime=Time.now(), location=elginfield)).alt*u.deg).value)
			sunaz = "{:.1f}".format((get_sun(Time.now()).transform_to(AltAz(obstime=Time.now(), location=elginfield)).az*u.deg).value)
			sunalt = "{:.1f}".format((get_sun(Time.now()).transform_to(AltAz(obstime=Time.now(), location=elginfield)).alt*u.deg).value)

			log_path = 'd:/Weather/Logs/'
			image_path = 'd:/Weather/Images/'

			# desktoppath = os.path.join(os.path.join(os.environ['USERPROFILE']), 'Desktop')
			# f = open(desktoppath + '\\weather-current.log', 'w')
			print(log_path)
			print(log_path + 'weather')
			f = open(log_path + 'weather-current.log', 'w')
			print('File opened')
			fmt = '%10s%9s%3s%2s%2s%7s%7s%7s%7s%4s%7s%4s%2s%2s%6s%13s%2s%2s%2s%2s%2s%2s'
			print(datenow)
			print(timenow)
			print(t_in)
			print(t_out)
			print(t_obs)
			print(v_wnd)
			f.write(fmt % (datenow, timenow, '.00', 'C', 'K', str(t_in), str(t_out), str(t_obs), str(v_wnd),\
			 str(h_out), str(dew), '000', '0', '0', '0001', '1', '1', '1', '1', '0', '0', '0')) #, str(sunaz), str(sunalt), str(moonaz), str(moonalt)))
			# f.write(timenow + ' ' + str(baro) + ' ' + str(t_in) + ' ' + str(t_out) + ' '\
			# 	+ str(t_obs) + ' ' + str(h_in) + ' ' + str(h_out) + ' ' + str(v_wnd) + ' '\
			# 	+ str(v_wnd_av) + ' ' + str(d_wnd) + ' ' + str(rain) + ' ' + str(dew))
			print('File written')
			f.close()

			print('#####')
			print('Current Weather is...')
			print('#####')
			print(' ')
			print('Temperature: ' + str(t_out))
			print('Humidity: ' + str(h_out) + '%')
			print('Wind Speed: ' + str(v_wnd_av) + ' km/s')
			print('Wind Direction: ' + str(d_wnd) + ' degrees')
			# print(solar)

			# Create dashboard...


			utcoffset = (time.localtime().tm_hour-time.gmtime().tm_hour)*u.hour  # Eastern Standard Time

			t = dt.now()
			today = Time(t.strftime('%Y-%m-%d') + ' 23:59:59') - utcoffset
			midnight = Time(today)
			# midnight = Time('2020-01-27 00:00:00') - utcoffset

			delta_midnight = np.linspace(-12, 12, 1000)*u.hour
			current_times = midnight + delta_midnight
			current_frame = AltAz(obstime=current_times, location=elginfield)
			sunaltazs_current = get_sun(current_times).transform_to(current_frame)

			moon_current = get_moon(current_times)
			moonaltazs_current = moon_current.transform_to(current_frame)
			#t = Time(Time.now(),format='iso')
			t = dt.now()
			print("Moon...")
			tdec = int(t.strftime('%H'))+int(t.strftime('%M'))/60 + int(t.strftime('%S'))/3600

			if tdec > 12.0 and tdec < 24.0:
				toff = tdec - 24.0
			else:
				toff = tdec

			# print(toff)

			warnspd = 15
			shutspd = 25
			maxspeed = 30

			speed = v_wnd

			#spdang = 180.0 - (speed/maxspeed * 180.0)

			patches = []

			cmaparr = sliceCBar(maxspeed,'viridis')
			cloudblk, cloudx, cloudy, tempg = openCloudLog(log_path + 'CloudMonitor\\current.log')

			# fig, ax = plt.subplots()
			fig = plt.figure(constrained_layout=True, figsize=(12,6))

			gs = fig.add_gridspec(2, 3)
			ax_tandh = fig.add_subplot(3,1,1)

			ax_tandh.plot(cloudx, tempg, color='black')
			ax_tandh.set_xlim(min(cloudx), max(cloudx))
			ax_tandh.set_xlabel('Local Time')

			ax_tandh_b = ax_tandh.twinx().twiny()
			ax_tandh_b.pcolorfast(cloudblk, cmap='Blues_r', vmin=-20, vmax=-10, alpha=0.7)

			ax_tandh_b.set_xticklabels([])
			ax_tandh_b.set_xticks([])
			ax_tandh_b.set_yticklabels([])
			ax_tandh_b.set_yticks([])

			ax_tandh_c = ax_tandh.twinx()
			sin_x = np.arange(min(cloudx),max(cloudx),0.1)
			sin_y = np.sin(sin_x)
			# ax_tandh_c.plot(sin_x,sin_y)
			ax_tandh_c.set_ylabel('Humidity (%)')

			ax_tandh.set_zorder(10)
			ax_tandh.patch.set_visible(False)

			ax_tandh.set_ylabel('Temperature (C)')
			ax_tandh.set_title('Temperature, Humidity, and Cloud Cover')

			ax_spd = fig.add_subplot(3,3,4)

			# add a wedge
			for i in range(180):
				wedge = mpatches.Wedge([0,0], 0.1, i, i+0.75, width=0.05, ec="white", lw=5)
				patches.append(wedge)

			wedge_gap = mpatches.Wedge([0,0], 0.0855, -1, 180, width=0.021, ec="none", fc="white", lw=1)

			if speed > warnspd & speed < shutspd:
				spdcolor = 'orange'
			elif speed > shutspd:
				spdcolor = 'red'
			else:
				spdcolor = 'green'

			wedge_spd = mpatches.Wedge([0,0], 0.085, spdang(speed), 180, width=0.02, ec="none", fc=spdcolor, lw=1)

			wedge_warn = mpatches.Wedge([0,0], 0.085, spdang(warnspd)-1, spdang(warnspd)+1, width=0.02, ec="white", fc='orange', lw=1)
			wedge_shut = mpatches.Wedge([0,0], 0.085, spdang(shutspd)-1, spdang(shutspd)+1, width=0.02, ec="white", fc='red', lw=1)

			colors = np.linspace(0, 1, len(patches))
			collection = PatchCollection(patches, cmap=plt.cm.magma_r, alpha=0.75)
			collection.set_array(np.array(colors))
			ax_spd.add_collection(collection)
			ax_spd.add_patch(wedge_gap)
			ax_spd.add_patch(wedge_spd)
			ax_spd.add_patch(wedge_warn)
			ax_spd.add_patch(wedge_shut)

			fontname = 'bold_led_board-7.ttf' # Path to ttf file

			if speed > warnspd & speed < shutspd:
				fontcolor = 'orange'
			elif speed > shutspd:
				fontcolor = 'red'
			else:
				fontcolor = 'green'

			ax_spd.text(0, 0.0115, speed, horizontalalignment='center', \
				verticalalignment='center', fontweight='bold', color=fontcolor, fontproperties=fm.FontProperties(fname=fontname, size=30))

			for i in range(1,10):
				ax_spd.text(0.0915 * np.cos(np.radians(180/10*i)), 0.0915 * np.sin(np.radians(180/10*i)), str(int(maxspeed-maxspeed/10*i)), \
					horizontalalignment='center', verticalalignment='center', color='black', \
					fontweight='bold', rotation = rot_text(180/10*i), fontproperties=fm.FontProperties(fname=fontname, size=10))

			# Add wind direction indicator
			wndx = np.sin(wnddir)*0.027
			wndy = np.cos(wnddir)*0.027

			ax_spd.add_patch(mpatches.Circle((-0.12,0.07),0.025, fc='none', ec='black', ls=':'))
			ax_spd.text(-0.125,0.1,'N', fontproperties=fm.FontProperties(fname=fontname, size=6))
			ax_spd.text(-0.125,0.032,'S', fontproperties=fm.FontProperties(fname=fontname, size=6))
			ax_spd.text(-0.09,0.065,'E', fontproperties=fm.FontProperties(fname=fontname, size=6))
			ax_spd.text(-0.1575,0.065,'W', fontproperties=fm.FontProperties(fname=fontname, size=6))
			ax_spd.arrow(-0.12, 0.07, wndx, wndy, head_width=0.008, head_length=0.015, length_includes_head=True, shape='right', overhang=0.1, ec='green', fc='green', lw=1)

			plt.axis('equal')
			plt.axis('off')

			ax_tel = plt.subplot2grid((3,3),(1,1),rowspan=1,colspan=2)
			ax_tel.text(0,0.9, '----- R -----', fontproperties=fm.FontProperties(fname=fontname, size=10))
			ax_tel.text(0,0.75, 'Tin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.75, '-5.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0,0.65, 'Hin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.65, '25.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0,0.55, 'Tout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.55, t_out, color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0,0.45, 'Hout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.45, h_out, color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0,0.35, 'Light', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.35, '0.01', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0,0.25, 'Rain', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.15,0.25, 'No', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))


			ax_tel.text(0.35,0.9, '----- G -----', fontproperties=fm.FontProperties(fname=fontname, size=10))
			ax_tel.text(0.35,0.75, 'Tin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.75, '-5.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.35,0.65, 'Hin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.65, '25.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.35,0.55, 'Tout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.55, '-7.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.35,0.45, 'Hout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.45, '23.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.35,0.35, 'Light', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.35, '0.01', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.35,0.25, 'Rain', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.5,0.25, 'No', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))

			ax_tel.text(0.7,0.9, '----- B -----', fontproperties=fm.FontProperties(fname=fontname, size=10))
			ax_tel.text(0.7,0.75, 'Tin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.75, '-5.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.7,0.65, 'Hin', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.65, '25.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.7,0.55, 'Tout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.55, '-7.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.7,0.45, 'Hout', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.45, '23.0', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.7,0.35, 'Light', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.35, '0.01', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.7,0.25, 'Rain', fontproperties=fm.FontProperties(fname=fontname, size=8))
			ax_tel.text(0.85,0.25, 'No', color='green', fontproperties=fm.FontProperties(fname=fontname, size=8))

			plt.axis('off')

			# ax_t2 = fig.add_subplot(3,3,6)

			ax_cld = fig.add_subplot(3,1,3)
			minmoonalt = np.min(moonaltazs_current.alt*u.deg).value

			ax_cld.plot(delta_midnight, sunaltazs_current.alt, color='r', label='Sun')
			ax_cld.plot(delta_midnight, moonaltazs_current.alt, color=[0.75]*3, ls='--', label='Moon')
			# plt.scatter(delta_midnight, m33altazs_current.alt,
			#             c=m33altazs_current.az, label='M33', lw=0, s=8,
			#             cmap='viridis')
			ax_cld.fill_between([-12,12],-18,0, hatch='x', color='green', alpha=0.8)
			ax_cld.fill_between(delta_midnight, 0, 90,
							 sunaltazs_current.alt < -0*u.deg, color='0.5', zorder=0)
			ax_cld.fill_between(delta_midnight, 0, 90,
							 sunaltazs_current.alt < -18*u.deg, color='k', zorder=0)
			# plt.colorbar().set_label('Azimuth [deg]')

			plt.axvline(toff, color='orange')
			ax_cld.text(toff-0.32, 91.5,'Now', color='orange', size=10)

			plt.legend(loc='upper left')
			plt.xlim(-12, 12)
			plt.xticks((np.arange(13)*2-12))
			plt.ylim(-18, 90)
			plt.xlabel('Hours from EST Midnight')
			plt.ylabel('Altitude [deg]')

			# print(Time.now())

			plt.tight_layout()
			plt.savefig(image_path + 'weatherdashboard.png', dpi=200)
			uploadFileFTP(image_path + 'weatherdashboard.png', server, username, password)
			plt.close()

		except:
			pass

		wx_socket.close()
		sleep(30)

if __name__ == "__main__":
	main()