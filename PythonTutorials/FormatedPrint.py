name = "Ana"
age = 25

# percent like formatting
print(" Sister %s has %d years" % (name, age))

# string format
print(" Sister {} has {} years".format(name, age))

# string formatting based on tuple index
print(" Sister {0} has {1} years. She has {1} year with name {0}".format(name, age))

# string formatting based on dictionary matching
print(" Sister {nm} has {ag} years. She has {ag} year with name {nm}".format(nm=name, ag=age))

person_dict = {
    'name': name,
    'age': age,
    'job': 'Programmer'
}
# string formatting based on dictionary matching
print(" Sister {name} has {age} years. She has {age} year with name {name}".format(**person_dict))

# replace with variables
print(f" Sister {name:*^20} has {age:+} years. She has {age:.2f} year with name {name.upper()}")
print(f"I work as a {person_dict['job']}")

