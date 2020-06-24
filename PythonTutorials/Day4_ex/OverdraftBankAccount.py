from Day4_ex.NormalBankAccount import BankAccount, InsufficientFundsException

class SpecialBankAccount(BankAccount):
    def __init__(self, bank_name, balance, OverDraft):
        super().__init__(bank_name, balance)
        self.MaxOverdDrat = OverDraft

    def Witdraw(self, amt):
        if amt > self.MaxOverdDrat + self.balance:
            raise InsufficientFundsException(f"Can't exceed overdraft. New balance would be {self.balance - amt}, limit is {-self.MaxOverdDrat}")
        self.balance -= amt
        print(f"{self.bank_name} new balance:", self.balance)

