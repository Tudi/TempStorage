def my_square(a):
    print(a * a)


def my_square_ret(a):
    return a * a


my_square(2)

if my_square(2) == my_square_ret(2):
    print("Results are same")
else:
    print("First function does not return a value")
