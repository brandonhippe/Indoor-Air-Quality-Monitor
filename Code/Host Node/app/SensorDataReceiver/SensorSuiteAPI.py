#try:
#    from Tkinter import *
#except:
#    from tkinter import ttk
from tkinter import *
from tkinter import ttk
import sys
import os
from datetime import datetime
import tkinter.font as tkFont
from SensorSuiteAPI import *
import time
import re
import binascii
import struct
# ================================ Classes for reading and golding data logs ==========================================

class sample: # Sample Object structure
    def __init__(self, timestamp, measurement):
        self.timestamp = timestamp
        self.type = ['CO2', 'PM', 'Airflow'][measurement[0]]
        self.value = measurement[1]

class Mote(): # Mote Object Structure : Contains Multiple Sample Objects
    def __init__(self, Directory, Logname):
        self.Logname = Logname
        self.Directory = Directory
        self.Logfile = open(self.Directory + self.Logname, "r")

        self.MAC = self.Logfile.readline().split()[2]
        self.status = 'Disconnected'
        self.UID = None
        self.coord = None
        #self.Logfile.close()

    # load a single mote from logfile into ram, using motes MAC address:
    def LoadMote(self):
        self.Logfile = open(self.Directory + self.Logname, "r")
        Loglines = self.Logfile.readlines()
        self.status = Loglines[1].split()[2]
        self.coord = Loglines[2].split()[2]
        self.UID = Loglines[3].split()[3]
        self.CurrentDate = None
        self.dates = []             # arranging all dates string into an array
        self.timesInDate = []       # row = new date, col = timestamps that correspond to the date
        self.samples = {"CO2": [], "PM": [], "Airflow": []} 
        self.timestamp = 0
        
        for line in Loglines:
            word = line.split()
            #print word
            if len(word) > 0:
                if word[0] == "--":                                     # Update Current Date
                    self.CurrentDate = word[1]
                    self.dates.append(word[1])
                    self.timesInDate.append([])
                if word[0] != "~" and word[0] != "--" and word[1][:2] != 'ef':                  # Convert Logfile text into organized sample objects
                    payload = binascii.unhexlify(word[1])
                    if payload[0] != 0:
                        value = struct.unpack('<Bf', bytearray(payload))
                    else:
                        value = struct.unpack('<BHH', bytearray(payload))[:-1]

                    timedate = self.CurrentDate +':'+ word[0][0:-1]     # combine time stamp with current date
                    timestamp = DtSeconds(datetime.strptime(timedate, "%m/%d/%Y:%H:%M:%S"))
                    self.timestamp = max(timestamp, self.timestamp)
                    self.timesInDate[-1].append(timestamp)
                     
                    s = sample(timestamp, value)
                    self.samples[s.type].append(s)

        self.Logfile.seek(0)
        self.Logfile.close()

    # return sample that matches time stamp:
    def timeRange(self, From, to):
        toI = -1
        fromI = -1
        fromclosest = abs(self.timestamp[-1] - From)
        toclosest = abs(self.timestamp[-1] - to)

        for i in range(len(self.timestamp)):
            compare = abs(self.timestamp[i] - From)
            if compare < fromclosest:
                fromclosest = compare
                fromI = i
        for i in range(len(self.timestamp)):
            compare = abs(self.timestamp[i] - to)
            if compare < toclosest:
                toclosest = compare
                toI = i
        return[fromI,toI]
    # Return Samples that match SensorIn
    def Sensor(self, sensorIn):
        sensors = ('temp', 'humid', 'N2O', 'o2', 'co2', 'accel', 'wind', 'rain')
        self.array = []
        if sensorIn == sensors[0]:
            for sample in self.samples:
                self.array.append(sample.temp)
            return self.array
        elif sensorIn == sensors[1]:
            for sample in self.samples:
                self.array.append(sample.humid)
            return self.array
        elif sensorIn == sensors[2]:
            for sample in self.samples:
                self.array.append(sample.N2O)
            return self.array
        elif sensorIn == sensors[3]:
            for sample in self.samples:
                self.array.append(sample.o2)
            return self.array
        elif sensorIn == sensors[4]:
            for sample in self.samples:
                self.array.append(sample.co2)
            return self.array
        elif sensorIn == sensors[5]:
            for sample in self.samples:
                self.array.append(sample.accel)
            return self.array
        elif sensorIn == sensors[6]:
            for sample in self.samples:
                self.array.append(sample.wind)
            return self.array
        elif sensorIn == sensors[7]:
            for sample in self.samples:
                self.array.append(sample.rain)
            return self.array
        else:
            print ('Wrong sensor string selection')

class MeshNetwork():    # Mesh Network object structure : Contains multiple Mote objects
    def __init__(self, Directory):
        self.Dir = Directory
        self.Motes = []
        self.MACaddresses = []
        self.MoteFiles = []
        self.moteLastUpdate = []            # Save the last date modified for each log file in the directory
        addresses = []
        MAC_root = "00-17-0d-00-00"
        print("Data Storage Directory: " + self.Dir)
        for file in os.listdir(self.Dir):
            if file.endswith(".log"):
                addresses.append(MAC_root + "-" + file[0:2] + "-" + file[2:4] + "-" + file[4:6])
                self.MoteFiles.append(file)

        [self.MACaddresses.append(x) for x in addresses if x not in self.MACaddresses]
        print("Known Motes: " + str(self.MACaddresses))

        self.NumOfMotes = len(self.MACaddresses)

        for file in self.MoteFiles:
            self.moteLastUpdate.append(os.stat(self.Dir + file).st_mtime)
    # Find unique addresses, define mote object with them, and load each one using loadMote():
    def LoadMesh(self):

        print ('\nNumber of motes: ' + str(len(self.MACaddresses)))
        for i in range(len(self.MoteFiles)):
            print ('mote ' + str(i) + ' ---> MAC: ' + self.MACaddresses[i])

            self.Motes.append(Mote(self.Dir, self.MoteFiles[i])) # define mote object using MAC
            self.Motes[i].LoadMote()      # load mote into ram

    # Return mote object that matches MAC Adress:
    def MoteMAC(self, MACaddress):
        for i in range(len(self.Motes)):
            if self.Motes[i].MAC == MACaddress:
                return self.Motes[i]
    # Chack if any Mote logs have been updated: if they have Reload the corresponding Mote objects from updated Log file
    def UpdateMesh(self):
        files = self.MoteFiles
        updatedMotes = []
        print ('>>>>>>checking if UPDATED')
        for n in range(len(files)):
            checkLastdate = os.stat(self.Dir + files[n]).st_mtime
            if not (self.moteLastUpdate[n] == checkLastdate):
                self.moteLastUpdate[n] = checkLastdate
                if self.Motes[n].status == 'DISCONNECTED':
                    print ('         '  + 'turing mote into operational')
                    with open(self.Dir + self.Motes[n].Logname, 'r+') as f:
                        text = f.read()
                        text = re.sub('DISCONNECTED', 'OPERATIONAL', text)
                        f.seek(0)
                        f.write(text)
                        f.truncate()
                self.Motes[n].LoadMote()
                updatedMotes.append(self.Motes[n].MAC[len(self.Motes[n].MAC) - 8:len(self.Motes[n].MAC)])
        if updatedMotes == []:
            return []
        else:
            return updatedMotes
    # Check if 10 Mins passed since the motes file has been updated: If true write corresponding mote as DISCONNECTED
    def UpdateStatus(self):
        files = self.MoteFiles
        updatedMotes = []
        dt = datetime.now()
        print ('>>>>>>checking if disconnected')
        for n in range(len(files)):
            if self.Motes[n].status == 'OPERATIONAL':
                checkLastdate = os.stat(self.Dir + files[n]).st_mtime
                notUpdated = abs(DtSeconds(dt) - checkLastdate) > (10 * 60) #Test
                if notUpdated:
                    with open(self.Dir + self.Motes[n].Logname, 'r+') as f:
                        text = f.read()
                        text = re.sub('OPERATIONAL', 'DISCONNECTED', text)
                        f.seek(0)
                        f.write(text)
                        f.truncate()
                    self.Motes[n].LoadMote()
                    self.moteLastUpdate[n] = os.stat(self.Dir + files[n]).st_mtime
                    updatedMotes.append(self.Motes[n].MAC[len(self.Motes[n].MAC) - 8:len(self.Motes[n].MAC)])
        if updatedMotes == []:
            return []
        else:
            return updatedMotes

    # Check if any of the latest samples in any of the motes triggers an Alert
    def CheckAlert(self):
        sensors = ('temp', 'humid', 'N2O', 'o2', 'co2', 'accel', 'wind', 'rain')
        alerts = []
        alertMotes = []
        is_alerted = False
        for mote in self.Motes:
            lastSample = mote.samples[-1]
            # check low trigger or high trigger
            if (self.tempLimit[0] > lastSample.temp) or (self.tempLimit[1] < lastSample.temp):
                is_alerted = True
                if self.tempLimit[0] < lastSample.temp:
                    alerts.append('LOW_TEMP')
                    print ('> temp is bellow ' + self.tempLimit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_TEMP')
                    print ('> temp is above ' + self.tempLimit[1] + ' for mote ' + mote.MAC)
            elif (self.humidLimit[0] > lastSample.humid) or (self.humidLimit[1] < lastSample.humid):
                is_alerted = True
                if self.humidLimit[0] < lastSample.humid:
                    alerts.append('LOW_HUMID')
                    print ('> humid is bellow ' + self.humidLimit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_HUMID')
                    print ('> humid is above ' + self.humidLimit[1] + ' for mote ' + mote.MAC)
            elif (self.N2OLimit[0] > lastSample.N2O) or (self.N2OLimit[1] < lastSample.N2O):
                is_alerted = True
                if self.N2OLimit[0] < lastSample.N2O:
                    alerts.append('LOW_N2O')
                    print ('> N2O is bellow ' + self.N2OLimit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_N2O')
                    print ('> N2O is above ' + self.N2OLimit[1] + ' for mote ' + mote.MAC)
            elif (self.o2Limit[0] > lastSample.o2) or (self.o2Limit[1] < lastSample.o2):
                is_alerted = True
                if self.o2Limit[0] < lastSample.o2:
                    alerts.append('LOW_O2')
                    print ('> o2 is bellow ' + self.o2Limit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_O2')
                    print ('> o2 is above ' + self.o2Limit[1] + ' for mote ' + mote.MAC)
            elif (self.co2Limit[0] > lastSample.co2) or (self.co2Limit[1] < lastSample.co2):
                is_alerted = True
                if self.co2Limit[0] < lastSample.co2:
                    alerts.append('LOW_CO2')
                    print ('> CO2 is bellow ' + self.co2Limit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_CO2')
                    print ('> CO2 is above ' + self.co2Limit[1] + ' for mote ' + mote.MAC)
            elif (self.windLimit[0] > lastSample.wind) or (self.windLimit[1] < lastSample.wind):
                is_alerted = True
                if self.windLimit[0] > lastSample.wind:
                    alerts.append('LOW_WIND')
                    print ('> wind is bellow ' + self.windLimit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_WIND')
                    print ('> wind is above ' + self.windLimit[1] + ' for mote ' + mote.MAC)
            elif (self.rainLimit[0] < lastSample.rain) or (self.rainLimit[1] < lastSample.rain):
                is_alerted = True
                if self.rainLimit[0] < lastSample.rain:
                    alerts.append('LOW_WATER')
                    print ('> rain is bellow ' + self.rainLimit[0] + ' for mote ' + mote.MAC)
                else:
                    alerts.append('HIGH_WATER')
                    print ('> rain is above ' + self.rainLimit[1] + ' for mote ' + mote.MAC)
            if is_alerted:
                alertMotes.append(mote.MAC)
        return alerts + " --> " + alertMotes

    # set alert triggers: limits should be a two element tuple (low trigger, high trigger)
    def SetAlertTriggers(self, sensorIn, limits):
        sensors = ('temp', 'humid', 'N2O', 'o2', 'co2', 'accel', 'wind', 'rain')
        self.array = []
        if sensorIn == sensors[0]:
            self.tempLimit = limits
        elif sensorIn == sensors[1]:
            self.humidLimit = limits
        elif sensorIn == sensors[2]:
            self.N2OLimit = limits
        elif sensorIn == sensors[3]:
            self.o2Limit = limits
        elif sensorIn == sensors[4]:
            self.co2Limit = limits
        elif sensorIn == sensors[5]:
            self.accelLimit = limits
        elif sensorIn == sensors[6]:
            self.windLimit = limits
        elif sensorIn == sensors[7]:
            self.rainLimit = limits
        else:
            print ('Wrong sensor string selection')

# convert Datetime Object into UNIX seconds
def DtSeconds(dt):          #Conver DATETIME to UTC seconds
    epoch = datetime.utcfromtimestamp(0)
    utc_s = (dt - epoch).total_seconds()
    return utc_s
# convert Date Hours and Minutes to UNIX seconds
def DateToUTC(date, hours, mins):   # convert date hours and minutes to UTC seconds
    epoch = datetime.utcfromtimestamp(0)
    dt = datetime.strptime(date + ':' + str(hours) + ':' + str(mins) + ':00', "%m/%d/%Y:%H:%M:%S")
    utc_s = (dt - epoch).total_seconds()
    return utc_s
# convert UNIX seconds to DateTime string
def UTCtoDate(utc_s):   # Convert UTC seconds to date and time
    dt = datetime.utcfromtimestamp(utc_s).strftime("%m/%d/%Y:%H:%M:%S")
    return dt


if __name__ == '__main__':
    Data_Loc = sys.path[0] + "/DataOrganization/"
    print (Data_Loc)
    Mesh1 = MeshNetwork(Data_Loc)
    Mesh1.LoadMesh()
    print (Mesh1.Motes[0].dates)
    dt = datetime.now()
    epoch = datetime.utcfromtimestamp(0)
    utc_s = DtSeconds(dt)
    print (utc_s)
    print (datetime.utcfromtimestamp(utc_s - 36*3600))
    mote0 = Mesh1.Motes[0]

    [dateFrom , dateTo] = [DateToUTC(mote0.dates[0], 19, 39) + 5 * 60, DateToUTC(mote0.dates[1], 19, 41)]

    fromto = mote0.timeRange(dateFrom,dateTo)
    print (fromto)
    print (UTCtoDate(dateFrom) + '---->' + UTCtoDate(mote0.timestamp[fromto[0]]))
    print (UTCtoDate(dateTo) + '---->' + UTCtoDate(mote0.timestamp[fromto[1]]))

    #print mote0.timesInDate[0]
    #print mote0.timesInDate[-1]
    print (Mesh1.UpdateMesh())
    #Logfile = open(Data_Loc + '3223d3.log', "r") WHERE DID THIS CODE COME FROM?
    #for i in range(1000):
    #    print (i)
    #    Logfile.seek(0)
    #    print (Logfile.readlines()[-1])
    #    Logfile.seek(0)

        # while 1:
        #     where = Logfile.tell()
        #     line = Logfile.readline()
        #     if not line:
        #         time.sleep(1)
        #         Logfile.seek(where)
        #     else:
        #         print line  # already has newline

    #.replace(tzinfo=pytz.utc).timestamp()
    #Mesh1.Motes[0].timeRange((5,25,21,11,52,00),(5,0,0,0,0,0))


'''
#========================================= MESH NETWORK ================================================================
                                        /       |       \
                                      /         |         \
                                    /           |           \
#============================== MOTE_1 ======= MOTE_2 ====== MOTE_3 =======MOTE_N+1.. ========
                                  |             |            |
                                  |             |            |
                                  |             |            |
                            Samples:        Samples:        Samples:
                            Temp  = []      Temp  = []      Temp  = []
                            humid = []      humid = []      humid = []
                            light = []      light = []      light = [] 
                            etc.. = []      etc.. = []      etc.. = []
'''

# ======================================= GUI HELPER CLASS/FUNC ========================================================
# Object to hold Tkinter table Label and button layout
class MoteTable():
    def __init__(self, Parent, Headers, tableRows, tableCols, datelist):
        self.ParentFrame = Parent
        self.dates = datelist
        self.Column = []
        self.table = []
        self.headrow = []
        self.headcol = []

        if not(Headers == None):
            for header in Headers:
                self.headrow.append(Button(Parent, bg = 'light blue', text = header,relief = RAISED , borderwidth=3))
        if not (datelist == None):
            for date in self.dates:
                datestr =UTCtoDate(date[0])
                self.headcol.append(Button(Parent, text=datestr[0:datestr.find(':')], relief=RAISED, borderwidth=2, height = 0, pady=0, font=("Helvetica",8)))
                #print("Start")
                #print(datestr[0:datestr.find(':')]) #Used to find out if all of the dates in the log file are displayed in the terminal window
                #print("Stop")

        # Layout lables and buttons in Rows: Disable by inputting tableRowa = None
        if not (tableRows == None):
            for i in range(len(tableRows)):
                row = tableRows[i]
                Label_row = []
                for cell in row:
                    if cell == row[0]:
                        Label_row.append(Button(self.ParentFrame, text=cell, borderwidth=3, font=("Helvetica",8)))
                    elif cell == row[1]:
                            if cell=='OPERATIONAL':
                                Label_row.append(
                                    Label(Parent, font=("Helvetica",8), bg='DarkOliveGreen1', text=cell, relief=GROOVE, pady=1,
                                          borderwidth=2))
                            else:
                                Label_row.append(
                                    Label(Parent, font=("Helvetica",8), bg='salmon1', text=cell, relief=GROOVE, pady=1,
                                          borderwidth=2))
                    else:
                        Label_row.append(Label(Parent, font=("Helvetica",8), bg = 'gainsboro', text=cell, relief=SUNKEN, borderwidth=2, pady=1))
                self.table.append(Label_row)
            for i in range(len(self.table)):
                print (self.table[i][0]['text'])
        # Layout lables and buttons in Rows: Disable by inputting tableCols = None
        elif not (tableCols == None):
            for col in tableCols :
                Label_col = []
                for cell in col:
                    if cell == col[0]:
                        #Label_col.append(Label(Parent, font=("Helvetica",8), text=cell[cell.find(':')+1:len(cell)-3], relief=GROOVE, borderwidth=2, padx=2))
                        #Replaced the top commented out code with the code below so that the seconds could be seen in the timestamps
                        Label_col.append(Label(Parent, font=("Helvetica",8), text=cell[cell.find(':')+1:len(cell)], relief=GROOVE, borderwidth=2, padx=2))
                    else:
                        Label_col.append(Label(Parent, font=("Helvetica",8), bg = 'gainsboro', text=cell, relief=SUNKEN, borderwidth=1, padx=1))
                    self.table.append(Label_col)



    # Pack all tkinter labels and buttons into the frame to create a table
    def pack_in(self, horizontal):
        if horizontal:
            Grid.rowconfigure(self.ParentFrame, 0, weight=1)
            self.LayRow(self.headrow, 0,0)
            for n in range(len(self.table)):
                Grid.rowconfigure(self.ParentFrame, n+1, weight=1)
                self.LayRow(self.table[n], n + 1, 0)
        else:
            lastcol = 0
            #self.headcol[0].grid(row=0, column=0, columnspan= 13, sticky=NSEW) These are just test cases for how to configure the date headers
            #self.headcol[1].grid(row=0, column=13, columnspan= 13, sticky=NSEW)
            #self.headcol[2].grid(row=0, column=26, columnspan= 13, sticky=NSEW)
            #self.headcol[3].grid(row=0, column=39, columnspan= 13, sticky=NSEW)
            for n in range(len(self.table)):
                if n < len(self.headcol):
                    Grid.rowconfigure(self.ParentFrame, n, weight=1)
                    span = len(self.dates[n])*4 #This was *9 before
                    self.headcol[n].grid(row=0, column=lastcol, columnspan= span, sticky=NSEW)
                    print (self.ParentFrame.winfo_geometry())
                    lastcol = lastcol + span #before this was lastcol = span which is incorrect because the lastcol would not change

                self.Laycol(self.table[n], n, 1)

    # Pack single Row
    def LayRow(self, Label_row, row_num, startCol):
        for n in range(len(Label_row)):
            Label_row[n].grid(row = row_num, column=startCol+n, sticky=NSEW)

    # Pack single Column
    def Laycol(self, Label_col, col_num, startrow):
        for n in range(len(Label_col)):
            Grid.rowconfigure(self.ParentFrame, startrow+n, weight=1)
            Label_col[n].grid(row = startrow+n, column=col_num, sticky=NSEW)