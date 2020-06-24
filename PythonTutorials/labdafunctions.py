mysum = lambda x, y: x + y


def mysumx(x, y):
    return x + y


print(mysum(1, 2))
print(mysumx(1, 2))

wordlist = ['hello', 'world', 'bye', 'hi']
print(min(wordlist))

min_str = min(wordlist, key=len)
print(min_str)

min_str = min(wordlist, key=lambda word: word[1])
print(min_str)

value_list = [1, 2, 3, 4, 5, 6, 7]
AFilterResult = filter(lambda x: x > 5, value_list)
value_list.append(8)
print(list(AFilterResult))

persons = [
    {'name': 'Ana', "age": 25},
    {'name': "Pupu", "age": 26},
    {'name': "Lele", "age": 27},
]

filtered_persons = filter(lambda d: d['age'] > 25, persons)
print(list(filtered_persons))

map_res = map(lambda x: x ** 2, value_list)
print(list(map_res))

map_res = map(print, value_list)
for elem in map_res:
    pass

person_number_zip = zip(value_list, persons, range(20, 30, 1))
print(*person_number_zip)

keys = ['name', 'age']
values = [['Mike', 50], ['Ana', 40]]
dicts = map(lambda lst: dict(zip(keys, lst)), values)
print(*dicts)

sorted_persons = sorted(persons, key=lambda person: person['name'])
print(*sorted_persons)

print(*enumerate(value_list))
