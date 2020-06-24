Price_ListEur = [('gas', '3'), ('diesel', 7), ('petrol', 6)]
ExchangeRate = 4.75
PriceListRON = zip(map(lambda prod: prod[0], Price_ListEur),
                   map(lambda prod: int(prod[1]) * ExchangeRate, Price_ListEur))
print(*PriceListRON)

# ver 2
PriceListRON = map(lambda prod: (prod[0], int(prod[1]) * ExchangeRate), Price_ListEur)
print(*PriceListRON)
