#try:
#    from Tkinter import *
#except:
from tkinter import *
from tkinter import ttk
import tkinter.font as tkFont
from SensorSuiteAPI import *
from GUI_GRAPHING import *

# UPDATE LIVE MOTE TABLE
def UpdateTable(Table, macs):
    for mac in macs:
        print ('Checking Mac: ' + mac)
        for row in Table:
            if mac == row[0]['text']:
                for mote in MainMesh.Motes:
                    if mac == mote.MAC[len(mote.MAC) - 8:len(mote.MAC)]:
                        print ('             updating row mac ' + row[0]['text'] + ' with info from mote: ' + mote.MAC)
                        if mote.status == 'OPERATIONAL': row[1]['bg'] = 'DarkOliveGreen1'
                        else: row[1]['bg'] = 'salmon1'
                        row[1]['text'] = mote.status
                        row[2]['text'] = mote.temp[-1]
                        row[3]['text'] = mote.humid[-1]
                        row[4]['text'] = mote.N2O[-1]
                        if mote.humid[-1] == 655.35:
                                row[3]['text'] = "--"
                        if mote.N2O[-1] == 655.35:
                                row[4]['text'] = "--"
                        #row[5]['text'] = mote.o2[-1] COMMENTING OUT SENSORS WE DON'T USE SO WE CAN READ 3 DATA VALUES FROM LOG FILE
                        #row[6]['text'] = mote.co2[-1]
                        #row[7]['text'] = mote.accel[-1]
                        #row[8]['text'] = mote.wind[-1]
                        #row[9]['text'] = mote.rain[-1]

sensorTracker = 0 #NEED TO HAVE THIS HERE SO THAT WHEN GUI IS FIRST LAUNCHED SENSORTRACKER WILL BE DEFINED!

# Change mote GRAPH
def changeGraph(sensor):
    global sensorTracker #Variable to track different sensors (useful for updating graphs)
    sensorTracker = 0
    hist_headers = ("Date", "Time", "Temp", "Humid", "N2O", "O2", "CO2", "Accel", "Wind", "Rain")
    mote =  MainMesh.Motes[ActiveMote]
    mac = mote.MAC[len(mote.MAC) - 8:len(mote.MAC)]
    title = ' samples for mote ' + mac

    if sensor == hist_headers[2]:
        title = 'Temperature(C)' + title
        sensorTracker = 0 #Used for tracking temp sensor when updating graph
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].temp)
    elif sensor == hist_headers[3]:
        title = 'Humidity(RH%)' + title
        sensorTracker = 1 #Used for tracking humid sensor when updating graph
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].humid)
    elif sensor == hist_headers[4]:
        title = 'N2O (ppm)' + title
        sensorTracker = 2 #Used for tracking N2O sensor when updating graph
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].N2O)
    elif sensor == hist_headers[5]:
        title = 'Oxygen (%)' + title
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].o2)
    elif sensor == hist_headers[6]:
        title = 'Carbon-Dioxide (CO2 ppt)' + title
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].co2)
    elif sensor == hist_headers[7]:
        title = 'Vibration (Hz)' + title
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].accel)
    elif sensor == hist_headers[8]:
        title = 'Wind (M/s)' + title
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].wind)
    elif sensor == hist_headers[9]:
        title = 'Rain (depth level)' + title
        graphPanel.__init__(MainMesh, FF_GRAPH, title, ActiveMote, MainMesh.Motes[ActiveMote].rain)

    print (sensor)

# CREATE HISTORY TABLE
def GUI_History_Table(Motenum):
    global ActiveMote
    ActiveMote = Motenum
    print ('mote num ---> ' + str(Motenum))
    for widget in FFF_hist.winfo_children():
        widget.destroy()
    for widget in FFF_hist.winfo_children():
        widget.destroy()
    hist_col = []
    hist_headers = ("Date", "Time", "Temp", "Humid", "N2O")
    for n in range(len(hist_headers)):
        if not(n == 0 or n == 1):
            headerButtons[n-2]['command'] = lambda x=hist_headers[n]: changeGraph(x)

    for sample in MainMesh.Motes[Motenum].samples[-100:]:
        if sample.N2O == 655.35: #Checking for 0 in N2O
            sample.N2O = "--"
        if sample.humid == 655.35: #Checking for 0 in humid
            sample.humid = "--"
        col = [UTCtoDate(sample.timestamp),
               sample.temp,
               sample.humid,
               sample.N2O
               ] # Delete rows from right table, but only number section
        #print (UTCtoDate(sample.timestamp)) #prints the date and timestamp
        hist_col.append(col)
    H_Table = MoteTable(FFF_hist, None, None, hist_col, MainMesh.Motes[Motenum].timesInDate)
    H_Table.pack_in(False)

dir = sys.path[0] + "/DataOrganization/"
# load in mesh network
MainMesh = MeshNetwork(dir)
MainMesh.LoadMesh()
MainMotes = MainMesh.Motes

if MainMesh.NumOfMotes == 0:
#    raw_input("NO LOG FILES FOUND...")
    print("NO LOG FILES FOUND...")
    sys.exit()

ActiveMote = 0
# initialize GUI
root = Tk()
root.title("Sensor Suite Data Viewer")
#root.iconbitmap('')
root.geometry("1050x650")
root.state('zoomed')

#========================================== Intialize Frames ==========================================================
Frame_Setup = LabelFrame(root, text = 'setup',      relief = GROOVE, padx=10, pady=10, borderwidth=4)
Frame_Motes = LabelFrame(root, text = 'Mote View',  relief = GROOVE, padx=5, pady=5, borderwidth=4)
Frame_History = LabelFrame(root, text = 'Graph',  relief = GROOVE, padx=5, pady=5, borderwidth=4)

#========================================== Frame_Setup ===============================================================
Lab_Directory = Label (Frame_Setup,
                       text = "Directory:               {directory}".format( directory = dir[0:3] + " .... " + dir[-35:-1])
                       )
Lab_ChangeDir = Label(Frame_Setup, text = "Change Directory: ")
In_dir = Entry(Frame_Setup, width=50, borderwidth=2) #seperate button call and grip placement so you can call button object in other places

lab_numOfMotes = Label(Frame_Setup, text = "Number of Motes in network:   " + str(MainMesh.NumOfMotes))
lab_DataSpan = Label(Frame_Setup, text = "DateSpan of collected Data:   " + MainMesh.Motes[0].dates[0] + ' - ' + MainMesh.Motes[0].dates[-1])

#========================================== Frame_Motes ===============================================================
# CREATE SCROLLABLE FRAME/CANVAS
MoteFrame = Frame(Frame_Motes)
MoteFrame.pack(fill=BOTH, expand = 1, ipadx = 40)
# Create Canvas
Mcanvas = Canvas(MoteFrame)
Mcanvas.pack(side=LEFT, fill=BOTH, expand=1)
# scrollbar
Mscrollbar = ttk.Scrollbar(MoteFrame, orient=VERTICAL, command=Mcanvas.yview)
Mscrollbar.pack(side=RIGHT, fill=Y)
# config canvas
Mcanvas.configure(yscrollcommand=Mscrollbar.set)
# bind config
Mcanvas.bind('<Configure>', lambda e: Mcanvas.configure(scrollregion=Mcanvas.bbox("all")))
SecondFrame = Frame(Mcanvas)
Mcanvas.create_window((0,0), window=SecondFrame, anchor=NW, width = 700)

Headers  = ("Mote","Status", "Temp", "Humid", "N2O") #Deleting headers from left table
Mote_Rows = []
if MainMesh.NumOfMotes == 1:
    row = [MainMesh.Motes[0].MAC[len(MainMesh.Motes[0].MAC) - 8:len(MainMesh.Motes[0].MAC)], MainMesh.Motes[0].status,
           MainMesh.Motes[0].temp[-1],
           MainMesh.Motes[0].humid[-1],
           MainMesh.Motes[0].N2O[-1]
           ] # Delete column from mote view
    Mote_Rows.append(row)
else:
    for mote in MainMesh.Motes:
        row = [mote.MAC[len(mote.MAC) - 8:len(mote.MAC)], mote.status,
               mote.temp[-1],
               mote.humid[-1],
               mote.N2O[-1],
               #mote.o2[-1], Commenting out sensors we do not use at the moment. This code relates to the other motes present in the mesh network.
               #mote.co2[-1], mote.accel[-1], mote.wind[-1], mote.rain[-1]
               ]
        Mote_Rows.append(row)
print ('mote len = ' + str(len(Mote_Rows)))
M_Table = MoteTable(SecondFrame, Headers, Mote_Rows, None, None)
M_Table.pack_in(True)

# Customize Cells
for cell in M_Table.headrow:
    cell['font'] = ("Helvetica",8)


#========================================== Frame_History =============================================================
# CREATE SCROLLABLE FRAME/CANVAS
Grid.rowconfigure(Frame_History, 0, weight=0)
Grid.rowconfigure(Frame_History, 1, weight=1)
Grid.columnconfigure(Frame_History, 0, weight=0)
Grid.columnconfigure(Frame_History, 1, weight=1)
FF_hist = Frame(Frame_History, bg='Black')
FF_Headers = Frame(Frame_History)
FF_GRAPH = Frame(Frame_History)
# FF_hist.pack(side=RIGHT, fill=BOTH, expand = 1, anchor=W)
# FF_Headers.pack(side=LEFT,ipady= 95, anchor=NW)

FF_hist.grid(row = 0, column = 1,ipady=0, sticky= NSEW)
FF_Headers.grid(row = 0, column = 0, sticky= NSEW)

FF_GRAPH.grid(row = 1, column = 0, columnspan=2, sticky= NSEW)
# Create Canvas
Hcanvas = Canvas(FF_hist)
Hcanvas.pack(side=TOP, fill=BOTH, expand=1)
# scrollbar
Hscrollbar = ttk.Scrollbar(FF_hist, orient=HORIZONTAL, command=Hcanvas.xview)
Hscrollbar.pack(side=BOTTOM, fill=X)
# config canvas
Hcanvas.configure(xscrollcommand=Hscrollbar.set)
# bind config
Hcanvas.bind('<Configure>', lambda e: Hcanvas.configure(scrollregion=Hcanvas.bbox("all")))
FFF_hist = Frame(Hcanvas)
Hcanvas.create_window((0, 0), window=FFF_hist, anchor='nw', height=270)

hist_headers = ("Date", "Time", "Temp", "Humid", "N2O")# Deleting row of buttons of right table
headerButtons = []
for n in range(len(hist_headers)):
    ##Grid.rowconfigure(FF_Headers, n, weight=1)
    if n == 0 or n == 1:
        Label(FF_Headers, bg='light blue', text=hist_headers[n], relief=RAISED, borderwidth=2, pady=16,
              font=("Helvetica",10)).grid(
            row=n + 1, sticky=NSEW) # Change Format of buttons, pady change height of button
    else:
        headerButtons.append(Button(FF_Headers, bg='light blue',
                                    text=hist_headers[n], borderwidth=2, pady=16))
        headerButtons[-1].grid(row=n + 1, sticky=NSEW) # Keep pady the same as above

GUI_History_Table(ActiveMote)
graphPanel = InteractiveGraph(MainMesh,
                              FF_GRAPH,
                              'Temp' + ' samples for mote ' + MainMesh.Motes[ActiveMote].MAC[len(MainMesh.Motes[ActiveMote].MAC) - 8:len(MainMesh.Motes[ActiveMote].MAC)],
                              ActiveMote,
                              MainMesh.Motes[ActiveMote].temp)

# assign buttons
for i in range(len(M_Table.table)):
    print (M_Table.table[i][0]['text'] +' === '+ str(i))
    M_Table.table[i][0]['command'] = (lambda x=i: GUI_History_Table(x))

# for n in range(100):
#     Button(SecondFrame, text=str(n)).grid(row= n, column=0)
#     Label(SecondFrame, text='wow', relief=SUNKEN).grid(row= n, column=1)
# MOTE TABLE



root.after(33, graphPanel.updateGUI)

#========================================== Frame_History =============================================================
# FRAME PACKING
Grid.rowconfigure(root, 0, weight=0)
Grid.rowconfigure(root, 1, weight=5)
Grid.columnconfigure(root, 0, weight=2)
Grid.columnconfigure(root, 1, weight=10)



Frame_Setup.grid(pady=6, padx=6, row = 0, column = 0, sticky=NSEW)
Frame_Motes.grid(pady=6, padx=6, row = 1, column = 0, sticky=NSEW)
Frame_History.grid(pady=6, padx=8, row = 0, rowspan=2,  column = 1, sticky=NSEW)

# LABEL PACKING
Lab_Directory.grid(row = 0, column = 0, columnspan= 2, sticky=W)
Lab_ChangeDir.grid(row = 1, column = 0, )
In_dir.grid(row = 1, column = 1)
# lab_numOfMotes.grid(row = 2, column = 0, columnspan= 2, sticky=W) # Number of motes online code
lab_DataSpan.grid(row = 3, column = 0, columnspan= 2, sticky=W) # Data Span code


Grid.columnconfigure(Frame_Motes, 0, weight=10)
Grid.columnconfigure(Frame_Motes, 1, weight=10)
pady = 190
# Tre_MoteTable.grid(row=0,column=0, ipady=pady, sticky=NS)
# MoteScroll.grid(row=0,column=1, sticky=NS)

def updateGUI():
    graphPanel.updateGUI()
    root.after(500, updateGUI)
def updateData():
    MACS = MainMesh.UpdateMesh()
    if MACS != []:
        UpdateTable(M_Table.table, MACS)
        GUI_History_Table(ActiveMote) #Added this code in order to update history table
        Hcanvas.configure(scrollregion=Hcanvas.bbox("all")) #Used to update scroll bar in history frame
        Hcanvas.xview_moveto(1) #Set scroll bar to the right position when new data arrives
        graphPanel.updateGraph(sensorTracker) #Update the graph when new data gets logged
    root.after(2000, updateData)
def updateStatus():
    MACS = MainMesh.UpdateStatus()
    if MACS != []:
        UpdateTable(M_Table.table, MACS)
    root.after(1000*60, updateStatus)


root.after(500,updateGUI)
root.after(2000,updateData)
root.after(1000*60,updateData)
updateStatus()

def on_closing():
    root.quit() #before this was tkinter.quit()
    root.after_cancel(updateGUI)
    root.after_cancel(updateData)
    root.after_cancel(updateData)
    for widget in root.winfo_children():
        widget.destroy()
    root.destroy()
root.protocol("WM_DELETE_WINDOW", on_closing)

root.mainloop()



