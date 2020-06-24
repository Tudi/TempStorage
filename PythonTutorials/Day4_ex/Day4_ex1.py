import time


def CallDurationProfiler(func):
    def wrapper(*args, **kwargs):
        StartTime = time.time_ns()
        result = func(*args, **kwargs)
        EndTime = time.time_ns()
        print(f"Function {func.__name__} call duration milliseconds: %.2f" % ((EndTime - StartTime) / 1000 / 1000))
        return result

    return wrapper


@CallDurationProfiler
def WasteTime(secs=10):
    time.sleep(secs)


WasteTime(2)
