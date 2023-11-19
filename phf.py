import random
# inputs = ["GET", "HEAD", "POST", "PUT", "DELETE", 
#           "CONNECT", "OPTIONS", "TRACE"]

# int_inputs = [int.from_bytes(bytes(i, 'ascii'), 'little') for i in inputs]

# answers = list(range(8))

inps = [random.randrange(2**32) for i in range(100000)]

def is_phf(h, inputs):
    return len({h(x) for x in inputs}) == len(inputs)

def colls(h, inputs):
    return len({h(x) for x in inputs})

# print(next(m for m in range(9, 2**32) if is_phf(lambda x: x % m, int_inputs)))

def h(x, c):
    m = (x * c) % 2**32
    return m

# out = [0]*8
# idxs = list(h(x, 0x1b8b6e6d) for x in int_inputs)

# for i, idx in enumerate(idxs):
#     out[idx] = answers[i]

# print([inputs[i] for i in out])
# print(list(out[h(x, 0x1b8b6e6d)] for x in int_inputs))

best = float('-inf')
while best < len(inps):
    c = random.randrange(2**32)
    # max_idx = max(h(x, c) for x in int_inputs)
    cls = colls(lambda x: h(x, c), inps)
    if cls > best:
        print(cls, hex(c))
        best = cls


        
# for i in inputs:
#     print(i, " ")
#     for c in i[::-1]:
#         print(hex(ord(c))[2:], end=" ")
#     print(" ")
#     print(int.from_bytes(bytes(i, 'ascii'), 'little'))
#     print("\n")
