from itertools import product
from collections import defaultdict
from math import ceil


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

    groupings = [[i for i in components if components[i]["Type"] == g] for g in groupings]
    for i in range(len(groupings) - 1, -1, -1):
        if len(groupings[i]) == 0:
            groupings.pop(i)

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

            period = components[i]["Time (On)"] + components[i]["Time (Off)"]

            if "Typical Power Consumption (On)" in components[i]:
                power[items][i] += components[i]["Typical Power Consumption (On)"] * components[i]["Time (On)"] / period
            else:
                power[items][i] += components[i]["Voltage (On)"] * components[i]["Typical Current (On)"] * components[i]["Time (On)"] / period

            if "Typical Power Consumption (Off)" in components[i]:
                power[items][i] += components[i]["Typical Power Consumption (Off)"] * components[i]["Time (Off)"] / period
            else:
                power[items][i] += components[i]["Voltage (Off)"] * components[i]["Typical Current (Off)"] * components[i]["Time (Off)"] / period

            if calculated:
                del(components[i]["Time (On)"])
                del(components[i]["Time (Off)"])

        batterySize[items] = sum(power[items].values()) * 24

    powerSorted = sorted(power.keys(), key=lambda e: sum(power[e].values()))
    costSorted = sorted(cost.keys(), key=lambda e: cost[e])

    output = []
    output.append("Configurations sorted by lowest power consumption:")
    for i, config in enumerate(powerSorted):
        output.append(f"{i + 1}.")
        for item in config:
            output.append(f"    {components[item]['Type']}: {item} ({power[config][item] / sum(power[config].values()) * 100} %)")

        output.append(f"    Power Consumption: {sum(power[config].values())} watts")
        output.append(f"    Cost: ${cost[config]}")
        output.append(f"    Battery Size for 1 Day battery life: {batterySize[config]} watt-hours")
        output.append(f"    Battery Size for 1 Week battery life: {batterySize[config] * 7} watt-hours")
        output.append(f"    Battery Size for 1 Month battery life: {batterySize[config] * 365 / 12} watt-hours")
        output.append(f"    Battery Size for 1 Year battery life: {batterySize[config] * 365} watt-hours")
        minCells = ceil(batterySize[config] * 365 / 12.6)
        minCells -= minCells - ceil(minCells / 2) * 2
        output.append(f"    18650 Cells required for 1 year battery life: {minCells} cells")
        output.append(f"    Battery Life with minimum 18650 cells: {minCells * 12.6 / batterySize[config]} days")


    output.append("\nConfigurations sorted by lowest cost:")
    for i, config in enumerate(costSorted):
        output.append(f"{i + 1}.")
        for item in config:
            output.append(f"    {components[item]['Type']}: {item} ({power[config][item] / sum(power[config].values()) * 100} %)")

        output.append(f"    Power Consumption: {sum(power[config].values())} watts")
        output.append(f"    Cost: ${cost[config]}")
        output.append(f"    Battery Size for 1 Day battery life: {batterySize[config]} watt-hours")
        output.append(f"    Battery Size for 1 Week battery life: {batterySize[config] * 7} watt-hours")
        output.append(f"    Battery Size for 1 Month battery life: {batterySize[config] * 365 / 12} watt-hours")
        output.append(f"    Battery Size for 1 Year battery life: {batterySize[config] * 365} watt-hours")
        minCells = ceil(batterySize[config] * 365 / 12.6)
        minCells -= minCells - ceil(minCells / 2) * 2
        output.append(f"    18650 Cells required for 1 year battery life: {minCells} cells")
        output.append(f"    Battery Life with minimum 18650 cells: {minCells * 12.6 / batterySize[config]} days")

    if input("Write to file (y/n)? ") == 'y':
        with open("output.txt", "w") as f:
            f.write('\n'.join(output))

    print('\n'.join(output))


if __name__ == "__main__":
    main()