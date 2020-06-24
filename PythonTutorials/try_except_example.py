d = {}
try:
    raise ValueError
except:
    print("catched value error")

try:
    a = d['somekey']
except (KeyError, ValueError) as MyEx:
    print("Exception occured : ", MyEx)
    print("Empty line to separate the next exception")
    raise Exception
except Exception:
    print("Managed to raise an exception in an exception")
else:
    print("No error occured")
finally:
    print("Executes every time")
