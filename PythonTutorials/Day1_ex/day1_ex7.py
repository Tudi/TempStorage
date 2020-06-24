# list of numbers sorted ascending print the quantity of distinct elements in it

InputList = [1, 2, 3, 5, 7, 8, 8]
UniqueElements = []
for elem in InputList:
    if elem not in UniqueElements:
        UniqueElements.append(elem)

print("Number of distinct elements in it :", len(UniqueElements))
for elem in UniqueElements:
    print(elem)

# var 2
i = 0
counter = 0
while i < len(InputList):
    i += InputList.count(InputList[i])
    counter += 1
print("var 2 :", counter)

# var 3
prev = None
counter = 0
for elem in InputList:
    if elem != prev:
        counter += 1
        prev = elem
print("var 3 :", counter)