buff = []

for num in range(100):
    if num < 10:
        buff.append(f"0x0{num}")
    else:
        buff.append(f"0x{num}")


# for num in range(100):
#     if num < 10:
#         buff.append(f"0x0{num}U")
#     else:
#         buff.append(f"0x{num}U")

# buff = []

# for num in range(154):
#     buff.append(num)


b = []
for i in buff:
    b.append(int(i, base=16))
    # b.append(i)


m = []
for i in range(100):
    # b.append(int(i, base=16))
    m.append(i)

p = 0
d = []
for i in range(154):
    if int(hex(i), base=16) in b:
    # if hex(i) in buff:
        d.append(p)
        p += 1

    else:
        d.append(0)


# for i in b:
#     # b.append(int(i, base=16))
#     d.append(hex(i))


# b = []
# for i in buff:
#     # b.append(int(i, base=16))
#     b.append(i)

# print(buff)
# print()
# print(m)
# print()
# print(b)
# print()

print(len(d))
print(d)



for i in buff:
    print(d[int(i, base=16)])
    
    
