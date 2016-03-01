from ternary_tree import TernaryTree

t=TernaryTree()
print (t.myfunc())
t.set_id_and_increment("apple")
print(t.get_id("apple"))

t.set_id_and_increment("яблоко")
print(t.get_id("яблоко"))

t.dump_ids("_ids")