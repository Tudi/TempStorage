a = 234
SumDigits = 0
while(a>0):
    SumDigits += a % 10
    a = a // 10
print(SumDigits)