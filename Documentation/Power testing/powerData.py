from collections import defaultdict
import math


UNIT_PREFIXES = {'p': -12, 'n': -9, 'u': -6, 'm': -3}
BAT_MAH = 3250 # mAh
BAT_VOLT = 3.7 # V
BAT_NUM = 4
BAT_COULOMBS = BAT_NUM * BAT_MAH * 3600 / 1000 # Converts mAh to Coulombs
MAX_T = 10 # Hours
GOAL_DAYS = 365


def lcm(data):
    val = 1
    for n in data:
        val = int(val * n / math.gcd(val, n))


    return val


def calcDutyCycle(dutyCycles):
    # Calculate Micro Controller's duty cycle
    periods = [int(sum(p)) for p in dutyCycles]
    maxPeriod = lcm(periods)

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


def componentCurrent(component):
    component["Current"] = component["Charge Consumed (On)"]
    if component["Period"] - component["Period (minimum)"] != 0:
        component["Current"] += (component["Typical Power Consumption (Off)"] / BAT_VOLT * (component["Period"] - component["Period (minimum)"]))

    component["Current"] /= component["Period"]


def determineSampling(components, days):
    seconds = days * 86400
    i = 0

    while True:
        # Calculate battery life at current sampling rates

        totalCurrent = 0
        dutyCycles = []
        for c in components.keys():
            if c == "Micro Com":
                continue
            
            dutyCycles.append([components[c]["Period (minimum)"], components[c]["Period"] - components[c]["Period (minimum)"]])

        micro_comDuty = calcDutyCycle(dutyCycles)
        components["Micro Com"]["Period"] = sum(micro_comDuty)
        components["Micro Com"]["Period (minimum)"] = micro_comDuty[0]
        componentCurrent(components["Micro Com"])
        
        for c in components.keys():
            totalCurrent += components[c]["Current"]

        for c in components.keys():
            components[c]["Percent"] = components[c]["Current"] / totalCurrent

        batteryLife = BAT_COULOMBS / totalCurrent
        
        # If greater than desired, exit and return
        if batteryLife >= seconds:
            return batteryLife / 86400

        # Else, reduce sampling rate of highest percentage component and retry
        maxPercent = 0
        component = None
        for c in components.keys():
            if c == "Micro Com":
                continue

            if components[c]["Percent"] > maxPercent and components[c]["Period"] < MAX_T * 3600:
                component = components[c]
                maxPercent = components[c]["Percent"]

        if component is None:
            raise Exception("Cannot extend measurement periods enough to meet battery life requirements")
                
        if component["Period"] < 60:
            component["Period"] -= component["Period"] % 5
            component["Period"] += 5
        elif component["Period"] < 300:
            component["Period"] -= component["Period"] % 15
            component["Period"] += 15
        elif component["Period"] < 600:
            component["Period"] -= component["Period"] % 30
            component["Period"] += 30
        elif component["Period"] < 6000:
            component["Period"] -= component["Period"] % 60
            component["Period"] += 60
        else:
            component["Period"] += 300

        componentCurrent(component)
        i += 1


def main():
    # Open, read, and parse file
    with open("Power Consumption Test Results - Power Consumption.csv", newline='') as f:
        lines = [line.strip().strip(',') for line in f.readlines()]

    columnNames = {i: lines[0].split(',')[i] for i in range(len(lines[0].split(',')))}

    components = defaultdict(lambda:dict)
    for line in lines[1:]:
        line = line.split(',')

        if line[0] == "EXAMPLE DATA":
            continue

        components[line[0]] = {}
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

    for c in components.keys():
        if c == "Micro Com":
            continue

        components[c]["Period"] = components[c]["Period (minimum)"]
        componentCurrent(components[c])

    output = []
    # withoutAnem = determineSampling({c: components[c] for c in components.keys() if c != "Anemometer"}, GOAL_DAYS)

    # output.append(f"Battery Life without Anemometer: {withoutAnem:.2f} days")
    # for c in components:
    #     if c == "Anemometer":
    #         continue

    #     output.append(f"{c}: {components[c]['Name']}\n\tPeriod: {components[c]['Period']} sec\n\tPercent of total power: {components[c]['Percent']:.2%}")
        
    #     if c != "Micro Com":
    #         output.append(f"\tPercent of component power consumed in sleep: {(components[c]['Typical Power Consumption (Off)'] / BAT_VOLT) / components[c]['Current']:.2%}")

    # output.append("\n\n")

    withAnem = determineSampling(components, GOAL_DAYS)

    output.append(f"Battery Life with Anemometer: {withAnem:.2f} days")
    for c in components:
        output.append(f"{c}: {components[c]['Name']}\n\tPeriod: {components[c]['Period']} sec\n\tPercent of total power: {components[c]['Percent']:.2%}")
        
        if c != "Micro Com":
            output.append(f"\tPercent of component power consumed in sleep: {(components[c]['Typical Power Consumption (Off)'] / BAT_VOLT) / components[c]['Current']:.2%}")

    if input("Print Results (y/n)? ") == 'y':
        for o in output:
            print(o)

    if input("Write to file (y/n)? ") == 'y':
        # Write to file
        with open(f"{input('Enter extensionless output file name: ')}.txt", "w") as f:
            f.write('\n'.join(output))



if __name__ == "__main__":
    main()