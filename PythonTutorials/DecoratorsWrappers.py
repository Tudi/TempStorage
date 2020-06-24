import functools


def MyChainedFunctionCaller(func):
    @functools.wraps(func) # this will make the function name show correctly instead wrapper. Docs will be shown from original function and not the wrapper
    def wrapper(*args, **kwargs):
        print("Wrapper called - before function call")
        result = func(*args, **kwargs)
        print("Wrapper called - after function call")
        return result

    return wrapper


@MyChainedFunctionCaller
def say_hello():
    print("Hello")


@MyChainedFunctionCaller
def greet(name):
    print(f'Hello,{name}')


def Multiply(times=1):
    def wrapperExt(func):
        def wrapperInt(*args, **kwargs):
            result = func(*args, **kwargs)
            return times * result

        return wrapperInt

    return wrapperExt


@MyChainedFunctionCaller
def get_sum(a, b=0):
    return a + b


@Multiply(3)
def get_sum2(a, b=0):
    return a + b


say_hello()
greet("Jane")
s = get_sum(b=10, a=1)
print(s)

print(get_sum2(2, 3))
