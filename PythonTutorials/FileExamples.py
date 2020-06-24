f = open("Hello.py")

for line in f:
    print(line)

# read the whole file at once
f.seek(0)
print(f.read())

f.close()

# resource management in a block
with open("Hello.py") as f:
    print(f.tell())
    f.close()
