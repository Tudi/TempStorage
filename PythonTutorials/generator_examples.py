def gen_numbers():
    print("1")
    yield 1
    print("2")
    yield 2
    print("3")
    yield 3
    print("4")


it_numbers = gen_numbers()
print(next(it_numbers))
print()
print(next(it_numbers))
