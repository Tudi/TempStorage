import random


def GenRandom(max_nr):
    while True:
        yield random.randint(1, max_nr)


for i in GenRandom(10):
    print(i)
    if i == 9:
        break
