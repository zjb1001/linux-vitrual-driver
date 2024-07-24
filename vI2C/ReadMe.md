Just compiler it step by step:

# Compiler it
make

# Install the module into kernel
sudo insmod vI2C.ko

# Check the module install successfully
ls /dev | grep I2C
... here is your terminal display
vI2C

# Test it or make your own test
python ../testPy/testI2C.py
... Here is your terminal display
0x010x020x3... ...
