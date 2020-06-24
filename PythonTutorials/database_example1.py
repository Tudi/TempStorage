import sqlite3

conn = sqlite3.connect("persons.db")
cursor = conn.cursor()

cursor.execute("Create table if not exists persons (name,date_of_birth)")
#cursor.execute("Insert into persons values ('ana', '10-02-1998')")
conn.commit()

cursor.execute("select * from persons")
records = cursor.fetchall()
print(records)

name = 'Dana'
date_of_birth = '04-03-2016'

cursor.execute(f"Insert into persons values ('{name}','{date_of_birth}')")
cursor.execute(f"Insert into persons values (?,?)", (name, date_of_birth))  # this will perform an sql_escape

cursor.execute("select * from persons")
records = cursor.fetchall()
print(records)
