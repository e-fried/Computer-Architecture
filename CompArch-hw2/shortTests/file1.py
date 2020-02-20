import random as rd

hexList = ["0", "4", "8", "C"]
opList = ["r", "w"]
# make traces
for i in range(1, 1000):
    path = "C:\mivne\hw2\pythoneTests/t" + str(i) +"Trace"
    trace = open(path, 'w')
    for j in range(20):
        operation = rd.choice(opList)
        address = "0x" + ('%07x' % rd.randrange(12)).upper()+rd.choice(hexList)
        trace.write(operation+" "+address+"\n")


memCyc = 100
L1Cyc = 10
L2Cyc = 20
for i in range(1, 1000):
    path = "C:\mivne\hw2\pythoneTests/t" + str(i)+"Cmd"
    cmd = open(path, 'w')
    bSize = 4 #rd.randrange(2, 4)
    wrAlloc = rd.randrange(2)
    L1Assoc = rd.randrange(1, 4)
    L1Size = bSize+L1Assoc + rd.randrange(6)
    L2Assoc = rd.randrange(2, 6)
    L2Size = L1Size + L2Assoc + rd.randrange(3)
    vicCache = 1 #rd.randrange(2)
    cmd.write("./cacheSim " + "pythoneTests/t" + str(i)+"Trace" + " --mem-cyc " + str(memCyc)+
              " --bsize "+str(bSize)+" --wr-alloc " +str(wrAlloc)+ " --l1-size " +str(L1Size)+
              " --l1-assoc " +str(L1Assoc)+" --l1-cyc "+str(L1Cyc)+ " --l2-size " +str(L2Size)+
                " --l2-assoc " +str(L2Assoc)+ " --l2-cyc " +str(L2Cyc)+ " --vic-cache " +str(vicCache))
