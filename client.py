import select
import socket
import sys
import threading
from threading import Lock
import subprocess
from time import sleep

def readInput(sock):
    while True:
        sys.stdout.flush()
        sleep(0.001)
        rlist, _, _ = select.select([sys.stdin], [], [], 1.0)
        if len(rlist):
            msg = sys.stdin.readline()
            if len(msg) > 1:
                sock.send(bytes(msg.rstrip(), encoding='UTF-8'))
            else:
                print("> ", end="")

            

with socket.socket() as sock:
    sock.connect(('localhost', 1234))
    threading.Thread(target=readInput, args=(sock,)).start()
    while True:
        msg = str(sock.recv(4096), encoding='UTF-8').rstrip()

        if len(msg) > 0:
            if "close" in msg:
                subprocess.run(["sleep", "1"])
            else:
                print(msg, end="\n> ")
