# >>> case: VM_Aug_Add_i32
# >>> code
def fun(a: i32) -> i32:
    a += 1
    return a
# <<<


# >>> call
fun(0)# <<<


# >>> expected
1# <<<


