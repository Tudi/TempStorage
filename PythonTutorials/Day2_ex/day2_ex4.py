InputString = ['aa', 'aa', 'bb', 'cc']

OutDict = {}
for elem in InputString:
    OutDict[elem] = InputString.count(elem)

print(OutDict)

# var 2
appearances = {}
for elem in InputString:
    if elem not in appearances:
        appearances[elem] = 1
    else:
        appearances[elem] += 1
print(appearances)
