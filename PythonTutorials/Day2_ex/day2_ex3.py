d = {'times': 100,
     'name': 'George',
     'hobies': ['fishing', 'hiking']}
d['friends'] = ['Andrei', 'Mihai', 'Alina']
print(d)

d['friends'].sort()
print(d)

d['hobies'].remove('hiking')
print(d)

d.pop('times')
print(d)
