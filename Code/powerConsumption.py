from itertools import product


UNIT_PREFIXES = {'p': -12, 'n': -9, 'u': -6, 'm': -3}


def main():
    with open("Power Consumption Test.csv", newline='') as f:
        lines = [line.strip().strip(',') for line in f.readlines()]

    groupings = []
    dutyCycles = {}
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
            elif currentType not in dutyCycles:
                dutyCycles[currentType] = float(line[-1][:-1]) / 100
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

    power = {}
    cost = {}

    for items in product(*groupings):
        p, c = 0, 0
        for i in items:
            c += components[i]["Cost"]

            if "Typical Power Consumption (On)" in components[i]:
                p += components[i]["Typical Power Consumption (On)"] * dutyCycles[components[i]["Type"]]
            else:
                p += components[i]["Voltage (On)"] * components[i]["Typical Current (On)"] * dutyCycles[components[i]["Type"]]

            if "Typical Power Consumption (Off)" in components[i]:
                p += components[i]["Typical Power Consumption (Off)"] * (1 - dutyCycles[components[i]["Type"]])
            else:
                p += components[i]["Voltage (Off)"] * components[i]["Typical Current (Off)"] * (1 - dutyCycles[components[i]["Type"]])
            
        power[items] = p
        cost[items] = c

    powerSorted = sorted(power.keys(), key=lambda e: power[e])
    costSorted = sorted(cost.keys(), key=lambda e: cost[e])

    print("\nConfigurations sorted by lowest power consumption:")
    for i, config in enumerate(powerSorted):
        print(f"{i + 1}.")
        for item in config:
            print(f"    {components[item]['Type']}: {item}")

        print(f"    Power Consumption: {power[config]} watts")
        print(f"    Cost: ${cost[config]}")

    print("\nConfigurations sorted by lowest cost:")
    for i, config in enumerate(costSorted):
        print(f"{i + 1}.")
        for item in config:
            print(f"    {components[item]['Type']}: {item}")

        print(f"    Power Consumption: {power[config]} watts")
        print(f"    Cost: ${cost[config]}")


if __name__ == "__main__":
    main()