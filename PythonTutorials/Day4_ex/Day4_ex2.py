from Day4_ex.Employee import Employee
from Day4_ex.NormalBankAccount import BankAccount
from Day4_ex.OverdraftBankAccount import SpecialBankAccount

if __name__ == "__main__":
    try:
        bnk = BankAccount("ing", 2)
        bnk.Deposit(4)
        bnk.Witdraw(5)
        bnk.Witdraw(5)
    except ValueError as err:
        print(err)

    try:
        spcbnk = SpecialBankAccount('reiff', 5, 5)
        spcbnk.Deposit(10)
        spcbnk.Witdraw(10)
        spcbnk.Witdraw(20)
    except ValueError as err:
        print(err)

    try:
        spcbnk = SpecialBankAccount('otp', 5, 5)
        ana = Employee("ana", spcbnk, 10)
        ana.Deposit(10)
        ana.Witdraw(10)
        ana.Witdraw(20)
    except ValueError as err:
        print(err)
