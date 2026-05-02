buff = []

for num in range(25):
    if num < 10:
        buff.append(f"0x0{num}U")
    else:
        buff.append(f"0x{num}U")


# b = []
# for i in buff:
#     b.append(int(i, base=16))
# b.append(i)


# m = []
# for i in range(100):
#     # b.append(int(i, base=16))
#     m.append(i)

# p = 0
# d = []
# for i in range(154):
#     if int(hex(i), base=16) in b:
#         # if hex(i) in buff:
#         d.append(p)
#         p += 1

#     else:
#         d.append(0)


# b = []
# for i in buff:
#     # b.append(int(i, base=16))
#     b.append(i)

print(buff)
# print()
# print(m)
# print()
# print(b)
# print()

for i in buff:
    print(i.replace("'", ""), end=", ")

# for i in buff:
#     print(d[int(i, base=16)])
