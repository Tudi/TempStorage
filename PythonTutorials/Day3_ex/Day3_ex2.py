SearchedText = input("What string to search for in file: ")
FileName = input("Specify input file name : ")
f = open(FileName)
for line in f:
    if SearchedText in line:
        print(line)
f.close()