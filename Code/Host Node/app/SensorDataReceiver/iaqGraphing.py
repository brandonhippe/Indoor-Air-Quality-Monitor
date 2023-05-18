from SensorSuiteAPI import *
import sys
import tkinter as tk
import matplotlib.dates as mdates
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime, timedelta


class IAQGraph:
    def __init__(self):
        self.dir = sys.path[0] + "/DataOrganization/"

        # load in mesh network
        self.MainMesh = MeshNetwork(self.dir)
        self.MainMesh.LoadMesh()
        # initialize GUI

        self.window = tk.Tk()
        self.window.title("Indoor Air Quality")

        self.fig = Figure(figsize = (5, 4), dpi = 100)
        self.ax = {"CO2": self.fig.add_subplot(3, 1, 1), "PM": self.fig.add_subplot(3, 1, 2), "Airflow": self.fig.add_subplot(3, 1, 3)}

        # Put plot into canvas
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Button to trigger plot update
        example_button = tk.Button(self.window, text="Plot Example", command=self.plotExample)
        example_button.pack(side=tk.BOTTOM)

        self.window.state('zoomed')


    def plotExample(self):
        for mote in self.MainMesh.Motes:
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

                self.ax[type].plot(times, samples, label = mote.Logname.split('.')[0])

        for ax in self.ax.values():
            ax.legend()
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.relim()
            ax.autoscale_view()

        self.canvas.draw()


    def update(self):
        curr_day = datetime.now()
        last_day = curr_day - timedelta(days=1)

        for ax in self.ax.values():
            ax.cla()

        for mote in self.MainMesh.Motes:
            if 'example' in mote.Logname:
                continue
            
            for type in self.ax.keys():
                if len(mote.samples[type]) == 0:
                    continue
                
                samples = []
                times = []
                for s in mote.samples[type]:
                    t = datetime.utcfromtimestamp(s.timestamp)

                    if not (last_day <= t <= curr_day):
                        continue

                    samples.append(s.value)
                    times.append(t)

                self.ax[type].plot(times, samples, label = mote.Logname.split('.')[0])

        for ax in self.ax.values():
            ax.legend()
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.set_xlim(last_day, curr_day)
            ax.autoscale_view()

        self.canvas.draw()


if __name__ == '__main__':
    iaq_graph = IAQGraph()

    tk.mainloop()