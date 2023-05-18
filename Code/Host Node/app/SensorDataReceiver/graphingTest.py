import tkinter as tk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Create tkinter window
window = tk.Tk()
window.title("Data Plot")

# Create matplotlib figure and axis
fig = Figure(figsize=(5, 4), dpi=100)
ax = fig.add_subplot(111)

# Define initial data
x = [1, 2, 3, 4, 5]
y = [2, 4, 6, 8, 10]

# Plot initial data
line, = ax.plot(x, y)

# Create FigureCanvasTkAgg object
canvas = FigureCanvasTkAgg(fig, master=window)
canvas.draw()
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

# Function to update the plot
def update_plot():
    # Update the data (example: multiply y values by 2)
    updated_y = [val * 2 for val in y]

    # Update the plot
    line.set_ydata(updated_y)
    ax.relim()
    ax.autoscale_view()
    canvas.draw()

# Button to trigger plot update
update_button = tk.Button(window, text="Update Plot", command=update_plot)
update_button.pack(side=tk.BOTTOM)

# Start tkinter event loop
tk.mainloop()
