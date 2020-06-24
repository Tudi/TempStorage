print("Hello world")

print("this is line 2")  # comment needs 2 spaces after code :P

print("this is a ",
      "multi line",
      "function")

a = 100 + 200 * 300 \
    * 400  # multi line instruction
print("a is = ", a, " variable type is ", type(a), " variable pointer :", id(a))

mylist = [1, "a string", 3.2]
for i in mylist:
    print(i, type(i), dir(i))

if 1 > 0:
    print("always true")
    print("Multi line if block")
else:
    print("never true")

MultiLineStr = """this is 
a 
copy pasted string"""
print(MultiLineStr)

print("Inverted string:", MultiLineStr[::-1])

# help(int)  # you can run this, but it will spam a lot of lines

# empty comment line
