from SensorSuiteAPI import *
import sys
import tkinter as tk
import matplotlib.dates as mdates
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime


class IAQGraph:
    def __init__(self) -> None:
        self.dir = sys.path[0] + "/DataOrganization/"

        # load in mesh network
        self.MainMesh = MeshNetwork(self.dir)
        self.MainMesh.LoadMesh()
        # initialize GUI

        self.window = tk.Tk()
        self.window.title("Indoor Air Quality")

        self.fig = Figure(figsize = (5, 4), dpi = 100)
        self.ax = self.fig.add_subplot(111)
        self.line, = self.ax.plot([datetime.utcfromtimestamp(s.timestamp) for s in self.MainMesh.Motes[0].samples['CO2']], [s.value for s in self.MainMesh.Motes[0].samples['CO2']])

        # Format x-axis ticks
        self.ax.xaxis.set_major_formatter(mdates.DateFormatter("%m/%d/%Y:%H:%M:%S"))
        self.ax.xaxis.set_major_locator(mdates.AutoDateLocator())

        # Put plot into canvas
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Button to trigger plot update
        update_button = tk.Button(self.window, text="Update Plot", command=self.update)
        update_button.pack(side=tk.BOTTOM)

        self.window.state('zoomed')

    def update(self):
        self.line.set_xdata([datetime.utcfromtimestamp(s.timestamp) for s in self.MainMesh.Motes[0].samples['PM']])
        self.line.set_ydata([s.value for s in self.MainMesh.Motes[0].samples['PM']])
        self.ax.relim()
        self.ax.autoscale_view()
        self.canvas.draw()


if __name__ == '__main__':
    iaq_graph = IAQGraph()

    tk.mainloop()