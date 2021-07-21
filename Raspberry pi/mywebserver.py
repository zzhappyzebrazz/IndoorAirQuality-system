from flask import Flask, render_template, request, send_file, make_response
import sqlite3
import sys
import time
import json
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure 
import io
import numpy as np

#variable define
dbname = 'sDatabase.db'
sampleFreq = 5 #time in second
global numSamples
global device
#connect to my database
conn = sqlite3.connect('../sDatabase.db')
curs = conn.cursor()

#initial Flask
app = Flask(__name__)
    
#retrieve the last data stored in database
def getData():
    conn = sqlite3.connect('../sDatabase.db')
    curs = conn.cursor()
    for row in curs.execute("SELECT * FROM DEVICE1_data ORDER BY timestamp DESC LIMIT 1"):
        time = str(row[0])
        Dust = row[1]
        eCO2 = row[2]
        eTVOC = row[3]
    conn.close()
    return time, Dust, eCO2, eTVOC

#retrive history data by number of samples
def get_HistData(numSamples):
    conn = sqlite3.connect('../sDatabase.db')
    curs = conn.cursor()
    curs.execute("SELECT * FROM DEVICE1_data ORDER BY timestamp DESC LIMIT "+str(numSamples))
    data=curs.fetchall()
    dates=[]
    Dust=[]
    eCO2=[]
    eTVOC=[]
    for row in reversed(data):
        dates.append(row[0])
        Dust.append(row[1])
        eCO2.append(row[2])
        eTVOC.append(row[3])
    conn.close()
    return dates, Dust, eCO2, eTVOC

#get the maximun number of row in the database
def maxRowsTable():
    conn = sqlite3.connect('../sDatabase.db')
    curs = conn.cursor()
    for row in curs.execute("SELECT COUNT(Dust) from  DEVICE1_data"):
        maxNumberRows=row[0]
    conn.close()
    return maxNumberRows

def rCO2(eCO2):
    if 0 < eCO2 and eCO2 <= 400:
        rating = 'Exellent'
    if 400 < eCO2 and eCO2 <= 1000:
        rating = 'Fine'
    if 1000 < eCO2 and eCO2 <= 1500:
        rating = 'Moderate'
    if 1500 < eCO2 and eCO2 <= 2000:
        rating = 'Poor'
    if 2000 < eCO2 and eCO2 <= 5000:
        rating = 'Very Poor'
    if eCO2 > 5000:
        rating = 'Severe'
    return rating
    
def rTVOC(eTVOC):
    if 0 < eTVOC and eTVOC <= 100:
        rating = 'Exellent'
    if 100 < eTVOC and eTVOC <= 250:
        rating = 'Fine'
    if 250 < eTVOC and eTVOC <= 350:
        rating = 'Moderate'
    if 350 < eTVOC and eTVOC <= 450:
        rating = 'Poor'
    if 450 < eTVOC and eTVOC <= 500:
        rating = 'Very Poor'
    if eTVOC > 500:
        rating = 'Severe'
    return rating
    
def AQIcalc(Dust):
    if 0 < Dust and Dust <= 54:
        AQI = ((50-0)/(54-0))*(Dust - 0) + 0
        rating = 'GOOD'
    if 54 < Dust and Dust <= 154:
        AQI = ((100-51)/(154-55))*(Dust - 55) + 51
        rating = 'MODERATE'
    if 154 < Dust and Dust <= 254:
        AQI = ((150-101)/(254-155))*(Dust - 155) + 101
        rating = 'Unhealthy For Sensitive Groups'
    if 254 < Dust and Dust <= 354:
        AQI = ((200-151)/(354-255))*(Dust - 255) + 151
        rating = 'Unhealthy'
    if 354 < Dust and Dust <= 424:
        AQI = ((300-201)/(424-355))*(Dust - 355) + 201
        rating = 'Very Unhealthy'
    if 424 < Dust and Dust <= 604:
        AQI = ((500-301)/(604-425))*(Dust - 425) + 301
        rating = 'Hazardous'
    return int(AQI), rating
    
#set max of samples
numSamples = maxRowsTable()
if (numSamples > 201):
    numSamples = 200
    
#index route
@app.route("/")
def index():
    global device
    time, Dust, eCO2, eTVOC = getData()
    AQI, AQIrating = AQIcalc(Dust)
    CO2rating = rCO2(eCO2)
    TVOCrating = rTVOC(eTVOC)
    templateData = {
                    'time': time,
                    'Dust': Dust,
                    'eCO2': eCO2,
                    'eTVOC': eTVOC,
                    'numSamples' : numSamples,
                    'AQI' : AQI,
                    'AQIrating' : AQIrating,
                    'CO2rating' : CO2rating,
                    'TVOCrating' : TVOCrating
                    }
    return render_template('index.html', **templateData)

#request for number of samples
@app.route('/', methods = ['POST'])
def num_from_post():
    global numSamples
    numSamples = int(request.form['numSamples'])
    numMaxSamples = maxRowsTable()
    if(numSamples > numMaxSamples):
        numSamples = (numMaxSamples - 1)
    time, Dust, eCO2, eTVOC = getData()
    AQI, AQIrating = AQIcalc(Dust)
    CO2rating = rCO2(eCO2)
    TVOCrating = rTVOC(eTVOC)
    templateData = {
                    'time': time,
                    'Dust': Dust,
                    'eCO2': eCO2,
                    'eTVOC': eTVOC,
                    'numSamples' : numSamples,
                    'AQI' : AQI,
                    'AQIrating' : AQIrating,
                    'CO2rating' : CO2rating,
                    'TVOCrating' : TVOCrating
                    }
    return render_template('index.html', **templateData)

        
#plot number of samples 
@app.route('/plot/Dust')
def plot_Dust():
    global numSamples
    global device
    times, Dusts, eCO2s, eTVOCs = get_HistData(numSamples)
    ys = Dusts
    fig = Figure()
    mean = [np.mean(ys)]*numSamples
    axis = fig.add_subplot(1, 1, 1)
    axis.set_title("Dust density [ug/m3]")
    axis.set_xlabel("Samples")
    axis.grid(True)
    xs = range(numSamples)
    axis.plot(xs, ys)
    axis.plot(xs, mean, label = 'Mean', linestyle ='--')
    canvas = FigureCanvas(fig)
    output = io.BytesIO()
    canvas.print_png(output)
    response = make_response(output.getvalue())
    response.mimetype = 'image/png'
    return response
    
@app.route('/plot/eCO2')
def plot_eCO2():
    global numSamples
    times, Dusts, eCO2s, eTVOCs = get_HistData(numSamples)
    ys = eCO2s
    fig = Figure()
    mean = [np.mean(ys)]*numSamples
    axis = fig.add_subplot(1, 1, 1)
    axis.set_title("eCO2 level [ppm]")
    axis.set_xlabel("Samples")
    axis.grid(True)
    xs = range(numSamples)
    axis.plot(xs, ys)
    axis.plot(xs, mean, label = 'Mean', linestyle ='--')
    canvas = FigureCanvas(fig)
    output = io.BytesIO()
    canvas.print_png(output)
    response = make_response(output.getvalue())
    response.mimetype = 'image/png'
    return response    
    
@app.route('/plot/eTVOC')
def plot_eTVOC():
    global numSamples
    times, Dusts, eCO2s, eTVOCs = get_HistData(numSamples)
    ys = eTVOCs
    fig = Figure()
    mean = [np.mean(ys)]*numSamples
    axis = fig.add_subplot(1, 1, 1)
    axis.set_title("eTVOC level [ppm]")
    axis.set_xlabel("Samples")
    axis.grid(True)
    xs = range(numSamples)
    axis.plot(xs, ys)
    axis.plot(xs, mean, label = 'Mean', linestyle ='--')
    canvas = FigureCanvas(fig)
    output = io.BytesIO()
    canvas.print_png(output)
    response = make_response(output.getvalue())
    response.mimetype = 'image/png'
    return response  
    
if __name__ == "__main__":
    app.run(host='0.0.0.0', port = 70, debug = False)
#Execute program
#main()
        
