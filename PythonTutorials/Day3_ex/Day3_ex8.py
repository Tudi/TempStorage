def GenDistinct(iterable):
    Uniques = set()
    for elem in iterable:
        if elem not in Uniques:
            Uniques.add(elem)
            yield elem


value_list = [1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6]
for i in GenDistinct(value_list):
    print(i)

gen_comp = (elem for elem in set(value_list))
for i in gen_comp:
    print(i)
