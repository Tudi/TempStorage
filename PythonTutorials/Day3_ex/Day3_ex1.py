x = input("Please provide x:")
y = input("Please provide y:")

try:
    res = int(x) / int(y)
    print("Result is ", res)
except (ZeroDivisionError, ValueError) as Err:
    print("Could not calculate result")
