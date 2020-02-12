
import time
import math as m

start = time.time()

def checksum(block):
    data_chk = []
    xor = '0'
    for line in block:
        for x in range(0, len(line) - 1, 2):
            xor = hex(int(xor, 16) ^ int(line[x], 16) ^ int(line[x + 1], 16))
        data_chk.append(xor)
        xor = '0'

    return data_chk


def stm_parser():

    data, block, d_chk = [], [], []
    addr = '0x08000000'
    start, i, x = 0, 0, 0

    with open("main.bin", "rb") as f:
        byte = f.read(1)
        while byte:
            for b in byte:
                data.append(hex(b))
            byte = f.read(1)

    while(start <= len(data)):
        block.append(data[start : start + 256])
        start = start + 256

    count = int(len(block))
    end = len(block) - 1
    i_addr = int(addr, 16)

    while(len(block[end]) % 256 != 0):
        block[end].append(hex(255))

    d_chk = checksum(block)

    print("Start Flashing")

    while(i < count):
        
        l_addr = hex(int(i_addr % 65536))
        h_addr = hex(int(i_addr / 65536))
        i_addr += 256

        a_chk = hex(int(l_addr, 16) ^ int(h_addr, 16))
        
        print("Sending WRITE MEMORY Command...")
        print("0x31", "0xCE")
        time.sleep(0.1)

        print("Sending Address...")
        print(h_addr, l_addr, a_chk)
        time.sleep(0.1)

        print("Sending Data...")
        # print(i + 1, block[i], d_chk[i])
        print(i + 1, "BLOCK", d_chk[i])
        time.sleep(0.1)

        i+=1
        print() 
    
    print("Done Flashing")  

stm_parser()


print('Time (ms):', 1000*(time.time() - start))
