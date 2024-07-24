# Python示例
import os

device_path = "/dev/vI2C"

with open(device_path, "wb") as device:
    # 写入数据
    device.write(bytearray([0x01, 0x02, 0x03]))
    device.write(bytearray([0x01, 0x02, 0x03]))
    # for _ in range(1024):
    #     device.write(bytearray([0x01, 0x02, 0x03]))
    

# To read from the device
with open(device_path, "rb") as device:
    data = device.read(1024)  # Read some data; specify size if needed
    print(data)