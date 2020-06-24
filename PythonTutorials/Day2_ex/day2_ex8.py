def HaveCommonInList(a, b):
    for elem in a:
        if elem in b:
            return True
    return False

def HaveCommonInList2(a, b):
    intersection = set(a) & set(b)
    if len(intersection) > 0:
        return True
    return False

l1 = [1, 2, 3]
l2 = [2, 3]
l3 = [4, 5]

print(HaveCommonInList(l1, l2))
print(HaveCommonInList(l2, l3))

print(HaveCommonInList2(l1, l2))
print(HaveCommonInList2(l2, l3))
