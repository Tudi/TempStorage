class InsufficientFundsException(Exception):
    pass

class BankAccount:
    def __init__(self, bank_name, balance):
        self.bank_name = bank_name
        self.balance = balance

    def Witdraw(self, amt):
        if amt > self.balance:
            raise InsufficientFundsException(f"Not enough money. Can't spend {amt} from {self.balance}")
        self.balance -= amt
        print(f"{self.bank_name} new balance:", self.balance)

    def Deposit(self, amt):
        if amt < 0:
            raise InsufficientFundsException("Lifehack detected")
        self.balance += amt
        print(f"{self.bank_name} new balance:", self.balance)
