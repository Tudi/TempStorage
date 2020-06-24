class MyClass:
    GlobalAttribute = 1  # all object instances will have this. Might get copied or referenced

    def __init__(self, a):
        self.InstanceAttribute = a  # only this object instance will have this attribute


class Person:
    TYPE = {'employee', 'student'}
    __TYPES = {'employee', 'student'}
    count = {}

    # define a list of attributes that can be assigned to this class. No more dynamic adding of new attributes
    __slots__ = ['name', 'age', 'energy_level', '_type', 'eye_color']

    def __init__(self, p_name, p_age, p_type):
        # if p_type not in self.__TYPES:
        #   raise Exception("Invalid type")
        self.name = p_name
        self.age = p_age
        self.energy_level = 0
        self.type = p_type
        self.increase_count(p_type)

    def sleep(self, hours):
        self.energy_level += hours * 10

    @classmethod
    def increase_count(cls, type):
        cls.count[type] = cls.count.get(type, 0) + 1

    # has access to class attributes only after at least 1 object has been created
    @classmethod
    def stats(cls, type):
        return Person.count.get(type, 0)

    # does not have access to self or global class
    @staticmethod
    def static_example(a, b):
        print("Received :", a, b)

    # will have a new name. Can't directly call it
    def __PrivateMethod(self):
        print("Should not be able to call this method from outside")

    # only generates a small warning when calling it
    def _ProtectedMethod(self):
        print("Called a protected method")

    # example of getter and setter functions
    def set_type(self, value):
        if value not in self.__TYPES:
            raise Exception("Invalid type")
        self._type = value

    def get_type(self):
        return self._type

    type = property(get_type, set_type)

    # example for a getter function that is actually a hidden class
    _YearsOld = 2

    @property
    def YearsOld(self):
        print("called getter for _YearsOld")
        return self._YearsOld

    @YearsOld.setter
    def YearsOld(self, NewValue):
        print("called setter for _YearsOld")
        self._YearsOld = NewValue

    def __str__(self):
        return f"Person name={self.name} and age={self.age}"


# inheritance example
class Student(Person):
    type = 'student'

    def _ProtectedMethod(self):
        super()._ProtectedMethod()

    def __init__(self, p_name, p_age, p_year):
        super().__init__(p_name, p_age, self.type)
        self.StudyYear = p_year


if __name__ == '__main__':
    x = MyClass(2)
    print(type(x))
    ana = Person('Ana', 25, 'employee')

    # testing class shadowing attributes
    print(ana)
    print(ana.TYPE)
    #    ana.TYPE = {'error'}
    print(ana.TYPE)
    print(Person.TYPE)
    Person.TYPE = {'error2'}
    print(Person.TYPE)
    ana.eye_color = 'green'
    print(ana.eye_color)

    # testing member functions
    ana.sleep(2)
    print("Energy level : ", ana.energy_level)

    # testing classmethods -> will always call the declaration function and not an object function
    peter = Person('Peter', 26, 'employee')
    print(Person.stats(1))

    # static methods are like namespace functions
    ana.static_example(1, 2)

    # inheritnace example
    dana = Student('Dana', 25, 2)

    # example for a getter function
    dana.YearsOld = 1
    print("Looks like an attribute, acts like a function :", dana.YearsOld)
