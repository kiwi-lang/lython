# version=2
# > 
# >> code
fun(a, b, c=d)# <<

# >> error:: NameError: name 'fun' is not defined
# >> error:: fun is not callable
# >> error:: NameError: name 'a' is not defined
# >> error:: NameError: name 'b' is not defined
# >> error:: NameError: name 'd' is not defined

# > 
# >> code
def myfunction(a: f64, b: f64) -> f64:
    return a + b

def fun():
    return myfunction(1.0, 2.0)
# <<

# >> call
fun()# <<

