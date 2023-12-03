import select
import socket
import sys
import threading
from threading import Lock
import sys
import subprocess
from time import sleep

def readInput(sock):
    while True:
        sys.stdout.flush()
        sleep(0.001)
        rlist, _, _ = select.select([sys.stdin], [], [], 1.0)
        if len(rlist):
            msg = sys.stdin.readline().rstrip()
            if len(msg) > 1:
                sock.send(bytes(msg, encoding='UTF-8'))
                if msg == 'exit':
                    return
            else:
                print("> ", end="")

if len(sys.argv) != 3:
    print("Bad arguments given. Expected: python3 client.py [IP] [PORT]")
    exit(1)

with socket.socket() as sock:
    sock.connect((sys.argv[1], int(sys.argv[2])))
    thread = threading.Thread(target=readInput, args=(sock,))
    thread.start()
    while True:
        if not thread.is_alive():
            print('Exiting...')
            sys.exit(0)

        msg = str(sock.recv(4096), encoding='UTF-8').rstrip()

        if len(msg) > 0:
            if "close" in msg:
                # subprocess.run(["sleep", "1"])
                print("shutting down")
                exit()
            else:
                print(msg, end="\n> ")