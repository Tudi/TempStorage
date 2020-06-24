from Day4_ex.NormalBankAccount import BankAccount


class Employee:
    def __init__(self, name, bankacct: BankAccount, salary):
        self.salary = salary
        self.bankAccount = bankacct
        self.name = name

    def receive_salry(self):
        self.bankAccount.Deposit(self, self.salary)

    def Witdraw(self, amt):
        self.bankAccount.Witdraw(amt)

    def Deposit(self, amt):
        self.bankAccount.Deposit(amt)