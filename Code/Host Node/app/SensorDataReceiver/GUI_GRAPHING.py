try:
    from Tkinter import *
except:
    from tkinter import *
from SensorSuiteAPI import *
from matplotlib import pyplot as plt
import matplotlib.dates as mdates
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

# ======================================== Interactive Class ===============================================
class InteractiveGraph():
    def __init__(self, Mesh, MFrame, title, mote, Data):
    # ============================DESTROY GRAPH WIDGETS
        for widget in MFrame.winfo_children():
            widget.destroy()
    # ============================INITIALIZE UI FRAMES AND MESH
        self.FFrame = Frame(MFrame)
        self.MainMesh = Mesh
        self.Data = Data
        FFrame2 = Frame(MFrame)
        self.FFrame.pack()
        FFrame2.pack(side=BOTTOM, expand=1, fill=X, )
        self.Mnum = mote
        self.SPH = 6 #If 1 then span goes from 15mins to 1hr to 1hr to 3hr. Never goes above 3hrs.
    # ============================FILL TIMESTAMP GAPS
        realTime = self.MainMesh.Motes[self.Mnum].timestamp
        # filledTime = []
        #
        # sampleIntervalS = (3600)/self.SPH
        # smpleInterval = (realTime[0]-realTime[1])
        # samplelength = int((realTime[0]-realTime[-1])/smpleInterval)
        # filledTime = np.linspace(realTime[0],realTime[-1],samplelength)
        # filledData = [0]*int(samplelength)
        # print '>>>>>>SAMPLE LENGTH  ' + str(samplelength)
        # for i in range(len(realTime)):
        #     for n in range(len(filledTime)):
        #         if abs(realTime[i] - filledTime[n]) < smpleInterval*1.5:
        #             filledTime[n] = realTime[i]
        #             filledData[n] = self.Data[i]
        # for i in range(len(realTime)):  # Fill non existing time stamps based on 6 samples per hour
        #     if i == 0:
        #         filledTime.append(realTime[i])
        #     if abs(realTime[i] - filledTime[-1]) > sampleIntervalS:
        #         while abs(filledTime[-1] - realTime[i]) > sampleIntervalS: # check if sample exists after 10 mins
        #             filledTime.append(filledTime[-1] + sampleIntervalS)
        #             filledData.append(0)
        #     else:
        #         filledTime.append(realTime[i])
        #         filledData.append(self.Data[i])
        # filledTime.pop(0)
    # ============================== INITIALIZE DATA
        #self.Data =  filledData
        #self.timestamps = [datetime.utcfromtimestamp(d) for d in filledTime]

        self.timestamps = [datetime.utcfromtimestamp(d) for d in realTime]
        self.dateStamps = [UTCtoDate(d)[0:UTCtoDate(d).find(':')] for d in realTime]
    # ==============================ADD TIME SLIDER
        self.span = self.SPH * 3
        self.offsetSpan = int(self.span * 0.4)
        self.sections = len(self.timestamps)/self.offsetSpan - self.span/self.offsetSpan
        self.time_slide = Scale(FFrame2, from_=0, to=self.sections, orient = HORIZONTAL)
        #self.time_slide.pack(fill=X, expand=1) **Eliminated time slider
        self.time_slide.set(self.sections)
    # =============================ADD DATE DROP DOWN MENU
        self.clicked = StringVar()
        options = self.MainMesh.Motes[self.Mnum].dates
        self.clicked.set(options[-1])
        self.oldOption = self.clicked.get()
        #drop = OptionMenu(FFrame2, self.clicked, *options) **Eliminated drop down menu
        #drop.pack(side=RIGHT)
    # ===========================ADD SPAN CHOICE BUTTONS
        #L_Span_T = Label(FFrame2, text = "Span: ").pack(side=LEFT) **Eliminated span button
        #B_Span_dec = Button(FFrame2, text = '<', padx = 0,command = lambda : self.ChangeSpan('down') ).pack(side=LEFT)
        #self.L_Span = Label(FFrame2, text = str(self.span/6) + 'hrs', relief = SUNKEN, padx= 10, bg='White')
        #self.L_Span.pack(side=LEFT)
        #B_Span_inc = Button(FFrame2, text = '>', padx = 0, command = lambda : self.ChangeSpan('up')).pack(side=LEFT)
        #L_numSamples = Label(FFrame2, text = "  Number of Samples: " + str(len(self.timestamps))).pack(side=LEFT)
    #===========================INITIALIZE PLOT AND PLOTTING DATA
        #y = self.Data[0:self.span]
        #t = self.timestamps[0:self.span]
        #self.Data = [0, 1, 2, 3, 4, 5] #Testing custom data
        #self.timestamps = [0, 1, 2, 3, 4, 5] #Testing custom timestamps
        y = self.Data[len(self.Data)-100:len(self.Data)] #plotting last 10 data values in log file
        t = self.timestamps[len(self.Data)-100:len(self.Data)]
        self.FFigure = plt.figure(figsize=(9,4), dpi = 120)#Modify DPI to change size of Graph
        plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        self.FFigure.add_subplot(111).plot(t,y)                             # PLOT
        chart = FigureCanvasTkAgg(self.FFigure, self.FFrame)
        chart.get_tk_widget().grid(row=0,column=0)
        plt.grid()                                                          # ADD GRID
        plt.gcf().autofmt_xdate()                                           # DATE FORMAT
        self.FFigure.clear()

        self.oldpoint = 0
        #self.updateDate()
        self.title = title
        self.labels = ['Dates','Samples']
        #self.updateGraph(0)
# ========================== SPAN CHOICE BUTTON FUNCTION
    def ChangeSpan(self,amount):
        SPH = self.SPH
        # INCREASE SPAN
        if amount=='up':
            if self.span == (SPH * 3):      self.span = SPH * 5
            elif self.span == (SPH * 5):    self.span = SPH * 9
            elif self.span == (SPH * 9):    self.span = SPH * 12
            elif self.span == (SPH * 12):   self.span = SPH * 16
            elif self.span == (SPH * 16):   self.span = SPH * 24
            elif self.span == (SPH * 24):   self.span = SPH * 36
            elif self.span == (SPH * 36):   self.span = SPH * 48
            elif self.span == (SPH * 48):   self.span = SPH * 72
            elif self.span == (SPH * 72):   self.span = SPH * 168
            elif self.span == (SPH * 168):   self.span = SPH * 168
        else:   # DECREASE SPAN
            if self.span == (SPH * 3):      self.span = SPH * 3 #3
            elif self.span == (SPH * 5):    self.span = SPH * 3
            elif self.span == (SPH * 9):    self.span = SPH * 5
            elif self.span == (SPH * 12):   self.span = SPH * 9
            elif self.span == (SPH * 16):   self.span = SPH * 12
            elif self.span == (SPH * 24):   self.span = SPH * 16
            elif self.span == (SPH * 36):   self.span = SPH * 24
            elif self.span == (SPH * 48):   self.span = SPH * 36
            elif self.span == (SPH * 72):   self.span = SPH * 48
            elif self.span == (SPH * 168):   self.span = SPH * 72
        self.offsetSpan = int(self.span*0.4)
        self.sections = len(self.timestamps)/self.offsetSpan - self.span/self.offsetSpan
        self.time_slide['to'] = self.sections
        self.L_Span['text'] = str(self.span/6) + 'hrs'
        self.updateGraph(self.time_slide.get())
# ========================== FUNCTION FOR UPDATING GUI
    def updateGUI(self):
        if self.clicked.get()!=self.oldOption:
            self.updateDate()
            self.oldOption = self.clicked.get()
        else: self.oldOption = self.clicked.get()

        newpoint = self.time_slide.get()
        if self.oldpoint != newpoint:
            self.updateGraph(newpoint)
            self.oldpoint = newpoint
        else: self.oldpoint = newpoint
# ========================== FUNCTION FOR DATE CHOICE DROP DOWN MENU: Doesnt work perfectly
    def updateDate(self):
        dateInd = []
        for i in range(len(self.dateStamps)):
            if self.dateStamps[i] == self.clicked.get(): dateInd.append(i)
        newoption = min((dateInd[-1])/self.offsetSpan - 1, 0)
        self.time_slide.set(newoption)
        print (newoption)
# ========================== FUNCTION FOR UPDATING GRAPH: CALLED FROM updateGUI()
    def updateGraph(self, sensorTracker):
        print("start")
        print(sensorTracker)
        print("stop")
        #newStart = timeSlide*self.offsetSpan
        #newFinal = newStart + self.span +1
        if sensorTracker == 0: #Temp sensor
            self.Data = self.MainMesh.Motes[self.Mnum].temp
        if sensorTracker == 1: #Humid sensor
            self.Data = self.MainMesh.Motes[self.Mnum].humid
        if sensorTracker == 2: #N2O sensor
            self.Data = self.MainMesh.Motes[self.Mnum].N2O
        #self.Data = self.MainMesh.Motes[0].temp
        realTime = self.MainMesh.Motes[self.Mnum].timestamp
        self.timestamps = [datetime.utcfromtimestamp(d) for d in realTime]
        newStart = len(self.Data) - 100 #This is for only plotting the last 10 data values in the log file
        newFinal = len(self.Data)
        print ('newRange == ' + str(newStart) + '--' + str(newFinal))
        y = self.Data[newStart:newFinal]
        t = self.timestamps[newStart:newFinal]
        x = np.array(y)
        index_zero = np.where(x == 655.35)[0]
        print(index_zero)
        for i in sorted(index_zero, reverse=True):
            del t[i]
        y = x[x != 655.35]
        print(self.Data[len(self.Data)-1]) #To check to see if self.Data updates when a new value has been added to the log file
        self.FFigure.clear()
        #plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%d/%H:%M')) #Removing data overlap on graph
        self.FFigure.add_subplot(111).plot(t, y)
        plt.title(self.title)
        plt.xlabel(self.labels[0])
        plt.ylabel(self.labels[1])
        chart = FigureCanvasTkAgg(self.FFigure, self.FFrame)
        chart.get_tk_widget().grid(row=0, column=0)
        plt.gcf().autofmt_xdate()
        plt.grid()

# execute if script is run directly
if __name__ == '__main__':
    dir = sys.path[0] + "/DataOrganization/"

    # load in mesh network
    MainMesh = MeshNetwork(dir)
    MainMesh.LoadMesh()
    # initialize GUI
    root = Tk()
    graphPanel = InteractiveGraph(MainMesh, root, 0, 0, MainMesh.Motes[0].temp)
    #graphPanel = InteractiveGraph(MainMesh,root,0, MainMesh.Motes[0].wind) THIS WAS OLD CODE

    root.after(33, graphPanel.updateGUI)
    root.mainloop()

