import socket
from scapy.all import *

UDP_IP = "0.0.0.0"  # Lắng nghe trên mọi địa chỉ
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024)  # Nhận gói tin UDP
    print(f"Received message: {data.decode()} from {addr}")

    # Chuyển tiếp gói tin UDP đến máy C
    udp_c_ip = "192.168.200.129"  # Địa chỉ IP của máy C
    udp_c_port = 12345  # Cổng UDP mà máy C đang lắng nghe

    # Gửi gói tin đến máy C
    sock.sendto(data, (udp_c_ip, udp_c_port))
