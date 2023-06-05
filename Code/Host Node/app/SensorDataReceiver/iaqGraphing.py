from SensorSuiteAPI import *
import sys
import tkinter as tk
import matplotlib.dates as mdates
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime, timedelta
import matplotlib.animation as animation
import time
import matplotlib.pyplot as plt


from matplotlib import style
# style.use('ggplot')


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
        for title in self.ax.keys():
            self.ax[title].set_title(title)

        self.default_colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

        # Put plot into canvas
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Button to trigger plot update
        example_button = tk.Button(self.window, text="Plot Example", command=self.plotExample)
        example_button.pack(side=tk.BOTTOM)

        plot_button = tk.Button(self.window, text="Plot Data", command=self.plot)
        plot_button.pack(side=tk.BOTTOM)

        self.window.state('zoomed')

        #self.window.after(1000, self.update)


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
            ax.legend()
            # ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.relim()
            ax.autoscale_view()

        self.canvas.draw()





    def plot(self):
        #self.ax.clear()        
        self.update()
        for i, mote in enumerate([m for m in self.MainMesh.Motes if 'example' not in m.Logname]):
            
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
            #ax.legend()
            # ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.relim()
            ax.autoscale_view()

        self.canvas.draw()
        self.window.after(10000, self.update)









    def update(self):

        curr_day = datetime.now()
        last_day = curr_day - timedelta(days=1)
        # last_day = curr_day - timedelta(days=.1) #Time delta changed for Testing CHANGE MEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

        for ax in self.ax.values():
            #ax.cla() 
            ax.clear() 

        for i, mote in enumerate([m for m in self.MainMesh.Motes if 'example' not in m.Logname]):
            
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

                self.ax[type].plot(times, samples, label = mote.Logname.split('.')[0], color = self.default_colors[i])

        for title, ax in zip(self.ax.keys(), self.ax.values()):
            ax.set_title(title)
            ax.legend()
            # ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            ax.set_xlim(last_day, curr_day)
            ax.autoscale_view()

        self.canvas.draw()

        self.MainMesh = MeshNetwork(self.dir)
        self.MainMesh.LoadMesh()


        self.window.after(10000, self.update)


if __name__ == '__main__':
    iaq_graph = IAQGraph()

    #time.sleep(1000)
    #iaq_graph.clear()
    #iaq_graph.mainloop()
    tk.mainloop()