from collections import defaultdict
from copy import deepcopy


UNIT_PREFIXES = {'p': -12, 'n': -9, 'u': -6, 'm': -3}
BAT_MAH = 3500 # mAh
BAT_VOLT = 3.7 # V
BAT_NUM = 4
BATT_ENG = BAT_NUM * BAT_MAH * 3600 / 1000 * BAT_VOLT # Converts mAh to Coulombs and mult by bat voltage
MAX_T = 3600 * 10 # Seconds/hr * hours


def gcd(a, b, memo = {}):
    # Memoized calculation of greatest common denominator of two numbers
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
    # Calculate least common multiple from gcd
    while len(nums) >= 2:
        a = nums.pop(0)
        b = nums.pop(0)
        nums.append(a * b / gcd(a, b))

    return nums[0]


def calcDutyCycle(dutyCycles):
    # Calculate Micro Controller's duty cycle
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


def componentPower(component):
    component["Power"] = (component["Measurement Energy"] + (component["Period"] - component["Period (minimum)"]) * component["Typical Power Consumption (Off)"]) / component["Period"]


def determineSampling(components, days):
    seconds = days * 86400
    communicationChanged = False
    i = 0

    while True:
        if i == 267:
            print("Check")
        # Calculate battery life at current sampling rates
        if not communicationChanged:
            components["Communication"]["Period"] = float('inf')

        totalPower = 0
        dutyCycles = []
        for c in components.keys():
            if c == "Micro Controller" or c == "Communication":
                continue
            
            dutyCycles.append([components[c]["Period (minimum)"], components[c]["Period"] - components[c]["Period (minimum)"]])
            
            if not communicationChanged:
                components["Communication"]["Period"] = min(components["Communication"]["Period"], components[c]["Period"])

        components["Communication"]["Period"] = max(components["Communication"]["Period"], components["Communication"]["Period (minimum)"])
        componentPower(components["Communication"])

        microControllerDuty = calcDutyCycle(dutyCycles)
        components["Micro Controller"]["Period"] = sum(microControllerDuty)
        components["Micro Controller"]["Period (minimum)"] = microControllerDuty[0]
        componentPower(components["Micro Controller"])
        
        for c in components.keys():
            totalPower += components[c]["Power"]

        for c in components.keys():
            components[c]["Percent"] = components[c]["Power"] / totalPower

        batteryLife = BATT_ENG / totalPower
        
        # If greater than desired, exit and return
        if batteryLife >= seconds:
            return batteryLife / 86400

        # Else, reduce sampling rate of highest percentage component and retry
        maxPercent = 0
        component = None
        for c in components.keys():
            if c == "Micro Controller":
                continue

            if components[c]["Percent"] > maxPercent and components[c]["Period"] < MAX_T:
                component = components[c]
                maxPercent = components[c]["Percent"]

        if component is None:
            raise Exception("Cannot extend measurement periods enough to meet battery life requirements")

        communicationChanged = component == components["Communication"]
                
        if component["Period"] % 5 != 0:
            component["Period"] += 1
        elif component["Period"] < 60:
            component["Period"] += 5
        elif component["Period"] < 300:
            component["Period"] += 15
        elif component["Period"] < 600:
            component["Period"] += 30
        elif component["Period"] < 6000:
            component["Period"] += 60
        else:
            component["Period"] += 300

        componentPower(component)
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
        if "Charge Consumed (On)" in components[c]:
            components[c]["Measurement Energy"] = components[c]['Voltage (On)'] * components[c]["Charge Consumed (On)"]
        else:
            components[c]["Measurement Energy"] = components[c]["Typical Power Consumption (On)"] * components[c]["Period (minimum)"]

        if c == "Micro Controller":
            continue

        components[c]["Period"] = components[c]["Period (minimum)"]
        componentPower(components[c])

    output = []
    withoutAnem = determineSampling({c: components[c] for c in components.keys() if c != "Anemometer"}, 365)

    output.append(f"Battery Life without Anemometer: {withoutAnem:.2f} days")
    for c in components:
        if c == "Anemometer":
            continue

        output.append(f"{c}: {components[c]['Name']}\n\tPeriod: {components[c]['Period']} sec\n\tPercent of total power: {components[c]['Percent']:.2%}")
        
        if c != "Micro Controller":
            output.append(f"\tPercent of component power consumed in sleep: {components[c]['Typical Power Consumption (Off)'] / components[c]['Power']:.2%}")

    output.append("\n\n")

    # withAnem = determineSampling(components, 365)

    # output.append(f"Battery Life with Anemometer: {withAnem:.2f} days")
    # for c in components:
    #     output.append(f"{c}: {components[c]['Name']}\n\tPeriod: {components[c]['Period']} sec\n\tPercent of total power: {components[c]['Percent']:.2%}")

    if input("Print Results (y/n)? ") == 'y':
        for o in output:
            print(o)

    if input("Write to file (y/n)? ") == 'y':
        # Write to file
        with open(f"{input('Enter extensionless output file name: ')}.txt", "w") as f:
            f.write('\n'.join(output))



if __name__ == "__main__":
    main()