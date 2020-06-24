import datetime
from peewee import SqliteDatabase, Model, CharField, DateField

db = SqliteDatabase('persons.db')


class Person(Model):
    name = CharField()
    date_of_birth = DateField()

    class Meta:
        database = db

    def __str__(self):
        return f'{self.name} ({self.date_of_birth})'


def create_tables():
    db.connect()
    db.create_tables([Person])


def drop_tables():
    db.drop_tables([Person])


if __name__ == '__main__':
    create_tables()

    # insert example
    person1 = Person(name='Pupu', date_of_birth="01-02-2019")
    person1.save()

    person2 = Person()
    person2.name = 'lele'
    person2.date_of_birth = datetime.date.today()
    person2.save()

    persons = Person.select()
    # print(persons)
    # person = persons.first()
    for person in persons:
        print(person.id, person.name, person.date_of_birth)

    # select example
    pupu = Person().select().filter(name='Pupu').get()
    print("===\n The pup : ", pupu.name, "\n===")

    # update example
    person2.date_of_birth = datetime.date(2020, 6, 16)
    person2.save()

    # complicated select
    query = Person.select()
    query = query.where(Person.date_of_birth < datetime.date.today())  # add a new where condition
    persons = query.get()
    print("persons born in the past: ", persons)
    #    for person in persons:
    #        print(person.id, person.name, person.date_of_birth)

    # delete example
    person1.delete_instance()
    pupu = Person().select().filter(name='Pupu')
    print("number of existing pupus", pupu.count())

    # create in 1 line
    Person.create(name='John', date_of_birth='')
    John = Person().select().filter(name='John')
    print("john line : ", *John)

    drop_tables()
