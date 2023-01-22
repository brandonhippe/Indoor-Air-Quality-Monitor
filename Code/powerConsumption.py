from itertools import product
from collections import defaultdict
from math import ceil
import matplotlib.pyplot as plt


UNIT_PREFIXES = {'p': -12, 'n': -9, 'u': -6, 'm': -3}
MAX_T = 10800
POWER_GOAL = 12.6 / (365 * 24)


def gcd(a, b, memo = {}):
    if (a, b) not in memo:
        if (a == 0):
            memo[(a, b)] = b

        if (b == 0):
            memo[(a, b)] = a

        if (a == b):
            memo[(a, b)] = a

        if (a > b):
            memo[(a, b)] = gcd(a-b, b)
        else:
            memo[(a, b)] = gcd(a, b-a)

    return memo[(a, b)]


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


def totalPower(items, components, cost, power, batterySize):
    dutyCycles = []
    for i in items:
        cost[items] += components[i]["Cost"]
        
        if "Time (On)" in components[i] and "Time (Off)" in components[i]:
            dutyCycles.append([components[i]["Time (On)"], components[i]["Time (Off)"]])
            calculated = False
        else:
            components[i]["Time (On)"], components[i]["Time (Off)"] = calcDutyCycle(dutyCycles)
            calculated = True

        power[items][i] = calcPower(components[i])

        if calculated:
            del(components[i]["Time (On)"])
            del(components[i]["Time (Off)"])

    batterySize[items] = sum(power[items].values()) * 24
    return sum(power[items].values())


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

    shortestPeriod = min(components[c]["Time (On)"] for c in components if "Time (On)" in components[c])
    firstUnder = {}

    for i, g in enumerate(groupings[1:]):
        for item in g:
            x, y = [], []

            offTime = components[item]["Time (Off)"]
            for t in range(int(components[item]["Time (On)"]), MAX_T + 1):
                x.append(3600 / t)
                components[item]["Time (Off)"] = t - components[item]["Time (On)"]
                y.append(calcPower(components[item]))

                if item not in firstUnder and y[-1] < POWER_GOAL / (len(groupings) - 1) and ((t < 600 and t % 60 == 0) or (t % 300 == 0)):
                    firstUnder[item] = [t, x[-1], y[-1]]
                
            components[item]["Time (Off)"] = offTime

            axs[i, 0].loglog(x, y, label = item)
            axs[i, 1].loglog(y, x, label = item)

        x, y = [0, 3600 / shortestPeriod], [POWER_GOAL / (len(groupings) - 1)] * 2
        axs[i, 0].loglog(x, y, 'r--', label = "Goal Avg Power/Item")
        axs[i, 1].loglog(y, x, 'r--', label = "Goal Avg Power/Item")
        axs[i, 0].legend()
        axs[i, 1].legend()


    plt.get_current_fig_manager().window.state('zoomed')
    fig.show()

    for item in list(components.keys()):
        if item in firstUnder:
            components[item]["Time (Off)"] = firstUnder[item][0] - components[item]["Time (On)"]
        elif components[item]["Type"] != "Micro Controllers":
            del components[item]

            for g in groupings:
                if item in g:
                    g.pop(g.index(item))

    power = defaultdict(lambda: defaultdict(lambda: 0))
    cost = defaultdict(lambda: 0)
    batterySize = {}

    for items in product(*groupings):
        items = tuple(sorted(items, key=lambda e: "Time (On)" not in components[e] and "Time (Off)" not in components[e]))

        p = totalPower(items, components, cost, power, batterySize)

    powerSorted = sorted(power.keys(), key=lambda e: sum(power[e].values()))

    output.append("Configurations sorted by lowest power consumption:")
    for i, config in enumerate(powerSorted):
        output.append(f"{i + 1}.")
        for item in config:
            output.append(f"    {components[item]['Type']}: {item}")
            if "Time (On)" in components[item]:
                output.append(f"        Time On: {components[item]['Time (On)']} seconds")
                output.append(f"        Time Off: {components[item]['Time (Off)']} seconds")
                
            output.append(f"        Percent of Total Power: {power[config][item] / sum(power[config].values()) * 100} %")

        output.append(f"    Power Consumption: {sum(power[config].values())} watts")
        output.append(f"    Cost: ${cost[config]}")
        output.append(f"    Battery Life on single 18650 cell: {12.6 / batterySize[config]} days")
        minCells = ceil(batterySize[config] * 365 / 12.6)
        minCells += minCells % 2
        output.append(f"    18650 Cells required for 1 year battery life (even only): {minCells} cells")
        output.append(f"    Battery Life with minimum (even) 18650 cells: {minCells * 12.6 / batterySize[config]} days\n")

    if input("Write to file (y/n)? ") == 'y':
        with open("output.txt", "w") as f:
            f.write('\n'.join(output))

    for o in output:
        print(o)


if __name__ == "__main__":
    main()