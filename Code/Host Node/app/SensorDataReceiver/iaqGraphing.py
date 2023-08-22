from SensorSuiteAPI import *
import sys
import tkinter as tk
import matplotlib.dates as mdates
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
from matplotlib import style

style.use('fivethirtyeight')
# style.use('seaborn-v0_8-talk')
# style.use('seaborn-v0_8-poster')
# style.use('seaborn-whitegrid')

TIMESTAMP_FORMAT = "%m/%d %I:%M"
PLOT_UNITS = {"CO2": "PPM", "PM": "µg/m3", "Airflow": "m/s"}


class IAQGraph:
    def __init__(self):
        self.dir = sys.path[0] + "/DataOrganization/"

        # load in mesh network
        self.MainMesh = MeshNetwork(self.dir)
        self.MainMesh.LoadMesh()
        # initialize GUI

        self.window = tk.Tk()
        self.window.title("Indoor Air Quality")

        self.fig = Figure(figsize=(5, 6), facecolor="white")  # Increased the figure size to accommodate padding
        self.ax = {"CO2": self.fig.add_subplot(311), "PM": self.fig.add_subplot(312), "Airflow": self.fig.add_subplot(313)}
        for title in self.ax.keys():
            self.ax[title].set_title(title)
            self.ax[title].set_facecolor("white")
            self.ax[title].grid(True)
            # self.ax[title].set_ylabel(PLOT_UNITS[title])
            # self.ax[title].tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)  # Remove x-axis ticks and labels

        self.ax['Airflow'].set_xlabel("Time of Day")
        self.fig.tight_layout(pad=1.0)  # Adjust the padding between subplots

        self.default_colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

        # Put plot into canvas
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Button to trigger plot update
        # example_button = tk.Button(self.window, text="Plot Example", command=self.plotExample)
        # example_button.pack(side=tk.BOTTOM)

        plot_button = tk.Button(self.window, text="Plot Data", command=self.update)
        plot_button.pack(side=tk.BOTTOM)

        self.window.state('zoomed')

        self.plotExample()
        
        self.batteryAlerts = set()


    def plotExample(self):
        #self.ax.clear()
        #self.update()
        #self.window.after(1000, self.update)
        #tk.after_cancel(self.window)
        for i, mote in enumerate(self.MainMesh.Motes):
            if 'example' not in mote.Logname:
                continue
            
            for type in self.ax.keys():
                if len(mote.samples[type]) == 0:
                    continue
                
                samples = []
                times = []
                for s in mote.samples[type]:
                    t = datetime.utcfromtimestamp(s.timestamp)
                    samples.append(s.value)
                    times.append(t)

                self.ax[type].plot(times, samples, label = mote.Logname.split('.')[0], color = self.default_colors[i])

        for title, ax in zip(self.ax.keys(), self.ax.values()):
            ax.set_title(title)
            self.ax[title].set_ylabel(PLOT_UNITS[title])
            ax.legend(loc="upper left")
            ax.xaxis.set_major_formatter(mdates.DateFormatter(TIMESTAMP_FORMAT))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.relim()
            ax.autoscale_view()
            ax.grid(True)
            ax.set_facecolor("white")

        self.ax['Airflow'].set_xlabel("Time of Day")

        self.canvas.draw()


    def update(self):
        end_time = datetime.now()
        start_time = end_time - timedelta(days=1)
        # start_time = end_time - timedelta(days=.1) #Time delta changed for Testing CHANGE MEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

        for ax in self.ax.values():
            #ax.cla() 
            ax.clear() 

        for i, mote in enumerate([m for m in self.MainMesh.Motes if 'example' not in m.Logname]):
            if mote.battery == 0 and mote.Logname not in self.batteryAlerts:
                self.batteryAlerts.add(mote.Logname)
                self.show_popup(mote)

            if mote.battery != 0 and mote.Logname in self.batteryAlerts:
                self.batteryAlerts.remove(mote.Logname)

            for type in self.ax.keys():
                if len(mote.samples[type]) == 0:
                    continue
                
                samples = []
                times = []
                for s in mote.samples[type]:
                    t = datetime.utcfromtimestamp(s.timestamp)

                    if not (start_time <= t <= end_time):
                        continue

                    samples.append(s.value)
                    times.append(t)

                self.ax[type].plot(times, samples, label = mote.Logname.split('.')[0] if mote.UID == 'None' else mote.UID, color = self.default_colors[i])

        for title, ax in zip(self.ax.keys(), self.ax.values()):
            ax.set_title(title)
            self.ax[title].set_ylabel(PLOT_UNITS[title])
            ax.legend(loc="upper left")
            ax.xaxis.set_major_formatter(mdates.DateFormatter(TIMESTAMP_FORMAT))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.set_xlim(start_time, end_time)
            ax.autoscale_view()
            ax.grid(True)
            ax.set_facecolor("white")

        self.ax['Airflow'].set_xlabel("Time of Day")

        self.canvas.draw()

        self.MainMesh = MeshNetwork(self.dir)
        self.MainMesh.LoadMesh()


        self.window.after(10000, self.update)

    
    def show_popup(self, mote):
        popup_window = tk.Toplevel()
        popup_window.title("BATTERY ALERT")
        
        # Add content to the popup window
        t = f"{mote.battery_timestamp.strftime(TIMESTAMP_FORMAT)}\nALERT: Mote {mote.Logname.split('.')[0]} "
        if mote.UID != 'None':
            t += f'({mote.UID})'

        t += ' LOW BATTERY'
        label = tk.Label(popup_window, text=t)
        label.pack(padx=20, pady=20)
        
        # Add a button to close the popup window
        close_button = tk.Button(popup_window, text="Close", command=popup_window.destroy)
        close_button.pack(pady=10)


if __name__ == '__main__':
    iaq_graph = IAQGraph()

    #time.sleep(1000)
    #iaq_graph.clear()
    #iaq_graph.mainloop()
    tk.mainloop()