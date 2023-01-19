from itertools import product
from collections import defaultdict
from math import ceil
import matplotlib.pyplot as plt


UNIT_PREFIXES = {'p': -12, 'n': -9, 'u': -6, 'm': -3}


def gcd(a, b):
    if (a == 0):
        return b

    if (b == 0):
        return a

    if (a == b):
        return a

    if (a > b):
        return gcd(a-b, b)
    else:
        return gcd(a, b-a)


def lcm(nums):
    while len(nums) >= 2:
        a = nums.pop(0)
        b = nums.pop(0)
        nums.append(a * b / gcd(a, b))

    return nums[0]


def calcDutyCycle(dutyCycles):
    periods = [sum(p) for p in dutyCycles]
    maxPeriod = lcm(periods[:])

    t = 0
    onTime = 0
    while t < maxPeriod:
        increment = 0
        for (o, _), p in zip(dutyCycles, periods):
            if t % p < o:
                increment = max(increment, o - (t % p))

        if increment == 0:
            increment = float('inf')
            for (o, _), p in zip(dutyCycles, periods):
                increment = min(increment, p - (t % p))
        else:
            onTime += increment

        t += increment

    return [onTime, maxPeriod - onTime]


def calcPower(component):
    power = 0
    period = component["Time (On)"] + component["Time (Off)"]
    power += component["Typical Power Consumption (On)"] * component["Time (On)"] / period
    power += component["Typical Power Consumption (Off)"] * component["Time (Off)"] / period
    return power


def main():
    with open("Power Consumption Test - Power Consumption.csv", newline='') as f:
        lines = [line.strip().strip(',') for line in f.readlines()]

    groupings = []
    components = {}

    prevLineBlank = True
    for line in lines:
        if len(line) == 0:
            prevLineBlank = True
        else:
            line = line.split(',')
            
            if prevLineBlank:
                currentType = line[0]
                groupings.append(line[0])
                columnNames = None
            elif columnNames is None:
                columnNames = {i: name for i, name in zip(range(len(line)), line)}
            else:
                components[line[0]] = {"Type": currentType}

                for i, field in enumerate(line[1:]):
                    if len(field) == 0:
                        continue

                    field = field.split(' ')

                    try:
                        components[line[0]][columnNames[i + 1]] = float(field[0])
                        
                        if len(field) > 1 and len(field[1]) > 1:
                            components[line[0]][columnNames[i + 1]] *= 10 ** UNIT_PREFIXES[field[1][0]]
                    except ValueError:
                        components[line[0]][columnNames[i + 1]] = ' '.join(field)

            prevLineBlank = False

    output = []
    for i in components:
        for p in ["On", "Off"]:
            if f"Typical Power Consumption ({p})" not in components[i]:
                components[i][f"Typical Power Consumption ({p})"] = components[i][f"Voltage ({p})"] * components[i][f"Typical Current ({p})"]

        if "Time (Off)" in components[i]:
            p1 = calcPower(components[i])

            powerInfo = components[i]["Typical Power Consumption (Off)"]
            components[i]["Typical Power Consumption (Off)"] = 0

            p2 = calcPower(components[i])

            components[i]["Typical Power Consumption (Off)"] = powerInfo

            output.append(f"{i}: Sleep power accounts for {(p1 - p2) / p1 * 100} % of total used power.")

    output.append('\n')

    groupings = [[i for i in components if components[i]["Type"] == g] for g in groupings]
    for i in range(len(groupings) - 1, -1, -1):
        if len(groupings[i]) == 0:
            groupings.pop(i)

    fig = plt.figure()
    gs = fig.add_gridspec(3, 2, hspace=0)
    axs = gs.subplots()
    for i in range(1, 3):
        for j in range(2):
            axs[i, j].sharex(axs[i - 1, j])
            axs[i, j].sharey(axs[i - 1, j])

    leftTitle, rightTitle = "", ""
    for g in groupings[1:]:
        leftTitle += f"{components[g[0]]['Type']} Power Consumption vs Sampling Rate (Logarithmic)\n"
        rightTitle += f"{components[g[0]]['Type']} Sampling Rate vs Power Consumption (Logarithmic)\n"

    axs[0, 0].set_title(leftTitle[:-1])
    axs[0, 1].set_title(rightTitle[:-1])

    axs[1, 0].set_ylabel("Power Consumption (Watts)")
    axs[1, 1].set_ylabel("Sampling Rate (Samples/Hour)")
    axs[2, 0].set_xlabel("Sampling Rate (Samples/Hour)")
    axs[2, 1].set_xlabel("Power Consumption (Watts)")

    for i, g in enumerate(groupings[1:]):
        for item in g:
            x, y = [], []

            offTime = components[item]["Time (Off)"]
            for t in range(int(components[item]["Time (On)"]), 3601):
                x.append(3600 / t)
                components[item]["Time (Off)"] = t - components[item]["Time (On)"]
                y.append(calcPower(components[item]))

            components[item]["Time (Off)"] = offTime

            axs[i, 0].loglog(x, y, label = item)
            axs[i, 1].loglog(y, x, label = item)

        axs[i, 0].legend()
        axs[i, 1].legend()


    plt.get_current_fig_manager().window.state('zoomed')
    fig.show()

    power = defaultdict(lambda: defaultdict(lambda: 0))
    cost = defaultdict(lambda: 0)
    batterySize = {}

    for items in product(*groupings):
        items = tuple(sorted(items, key=lambda e: "Time (On)" not in components[e] and "Time (Off)" not in components[e]))
        
        dutyCycles = []
        for i in items:
            cost[items] += components[i]["Cost"]
            
            if "Time (On)" in components[i] and "Time (Off)" in components[i]:
                dutyCycles.append([components[i]["Time (On)"], components[i]["Time (Off)"]])
                calculated = False
            else:
                components[i]["Time (On)"], components[i]["Time (Off)"] = calcDutyCycle(dutyCycles)
                calculated = True

            power[items][i] += calcPower(components[i])

            if calculated:
                del(components[i]["Time (On)"])
                del(components[i]["Time (Off)"])

        batterySize[items] = sum(power[items].values()) * 24

    powerSorted = sorted(power.keys(), key=lambda e: sum(power[e].values()))

    output.append("Configurations sorted by lowest power consumption:")
    for i, config in enumerate(powerSorted):
        output.append(f"{i + 1}.")
        for item in config:
            output.append(f"    {components[item]['Type']}: {item} ({power[config][item] / sum(power[config].values()) * 100} %)")

        output.append(f"    Power Consumption: {sum(power[config].values())} watts")
        output.append(f"    Cost: ${cost[config]}")
        output.append(f"    Battery Life on single 18650 cell: {12.6 / batterySize[config]} days")
        minCells = ceil(batterySize[config] * 365 / 12.6)
        minCells += minCells % 2
        output.append(f"    18650 Cells required for 1 year battery life: {minCells} cells")
        output.append(f"    Battery Life with minimum 18650 cells: {minCells * 12.6 / batterySize[config]} days")

    if input("Write to file (y/n)? ") == 'y':
        with open("output.txt", "w") as f:
            f.write('\n'.join(output))

    for o in output:
        print(o)


if __name__ == "__main__":
    main()