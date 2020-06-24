def OrderedStringList(*Strings):
    AllStrings = list(*Strings)
    UniqueStrings = set(AllStrings)
    OrderedUniqueStrings = sorted(UniqueStrings, key=lambda x: AllStrings.count(x), reverse=True)
    return OrderedUniqueStrings


StringsList = ('a', 'b', 'aa', 'b', 'a', 'a')
print(StringsList)
print("Frequency ordered unique list : ", *OrderedStringList(StringsList))
