import select
import socket
import subprocess
import sys
import threading
import sys
from time import sleep

def is_socket_closed(sock: socket.socket) -> bool:
    try:
        data = sock.recv(16, socket.MSG_DONTWAIT | socket.MSG_PEEK)
        if len(data) == 0:
            return True
    except BlockingIOError:
        return False
    except ConnectionResetError:
        return True
    except Exception as e:
        logger.exception("unexpected exception when checking if a socket is closed")
        return False
    return False

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

def recvall(sock):
    BUFF_SIZE = 4096
    data = b''
    while True:
        part = sock.recv(BUFF_SIZE)
        data += part
        if len(part) < BUFF_SIZE:
            break
    return data

if len(sys.argv) != 3:
    print("Bad arguments given. Expected: python3 client.py [IP] [PORT]")
    exit(1)

with socket.socket() as sock:
    sock.connect((sys.argv[1], int(sys.argv[2])))
    thread = threading.Thread(target=readInput, args=(sock,), daemon=True)
    thread.start()
    while True:
        if is_socket_closed(sock):
            print('Exiting...')
            sys.exit(0)

        if not thread.is_alive():
            print('Exiting...')
            sys.exit(0)

        msg = str(recvall(sock), encoding='UTF-8').rstrip()

        if len(msg) > 0:
            if msg == "close":
                print("\nClient will be shutdown in 5 seconds...")
                sleep(5)
                if sys.platform == 'win32':
                    subprocess.Popen(r"C:\Windows\System32\shutdown.exe /p /f", creationflags=subprocess.CREATE_NO_WINDOW)
                else:
                    subprocess.run(["shutdown", "0"], stdout = subprocess.DEVNULL)
                sys.exit(0)
            else:
                print(msg, end="\n> ")