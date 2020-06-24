def CommonElem(*LisOfLists):
    FirstList = LisOfLists[0]
    RestLists = LisOfLists[1:]
    for elem1 in FirstList:
        FoundCount = 0
        for ListItr in RestLists:
            if elem1 in ListItr:
                FoundCount += 1
        if FoundCount == len(LisOfLists) - 1:
            return True
    return False


def CommonElem2(*LisOfLists):
    if not LisOfLists:
        return False
    first_set = set(LisOfLists[0])
    for lst in LisOfLists[1:]:
        first_set.intersection_update(set(lst))
        if not first_set:
            return False
    return True

def HaveCommonInList2(a, b):
    intersection = set(a) & set(b)
    if len(intersection) > 0:
        return True
    return False

def CommonElem3(*LisOfLists):
    if len(LisOfLists) == 2:
        return HaveCommonInList2(LisOfLists[0],LisOfLists[1])
    return CommonElem3(set(LisOfLists[0]) & set(LisOfLists[1]), *LisOfLists[2:])


l1 = [1, 2, 3]
l2 = [2, 3]
l3 = [4, 5]
l4 = [3, 4]

print(CommonElem(l1, l2, l4))
print(CommonElem(l1, l2, l3, l4))

print(CommonElem3(l1, l2, l4))
print(CommonElem3(l1, l2, l3, l4))
