import socket
import time

SERVER_HOST = '127.0.0.3'
SERVER_PORT = 8081 # Bdel l port ila lزم

# L request li ghadi tjib lina l file kbir
request = (
    b"GET /large_file.bin HTTP/1.1\r\n"
    b"Host: localhost\r\n"
    b"Connection: close\r\n\r\n"
)

try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_HOST, SERVER_PORT))
    print("Kan sift l request...")
    client_socket.sendall(request)
    
    # Tsenna chwiya (0.1s) bach l server ybda ysifet
    print("Ghadi nqte3 l connection daba...")
    time.sleep(0.1)
    
    # Qte3 l connection f'أة
    client_socket.close()
    print("Connection tqet3at. Chof l server dyalk.")

except Exception as e:
    print(f"Error: {e}")
