# -GPIO-based-LED-Driver-in-Linux-
 Develop a char device driver to control RGB led display. To test the driver, a user-space application is written to display a lighting sequence of {OFF, R, G, B, R&amp;G, R&amp;B, G&amp;B, R&amp;G&amp;B} where each step of the lighting sequence is with a duration 0.5 second. The lighting sequence should be displayed repetitively and can be interrupted by any mouse button click. If there is a button click, the display should immediately move to the beginning of the sequence.






1) Give power to the board Intel Galileo Gen 2 and connect the ethernet and the USB from host to the board.
 

2) Connect the red, green, blue pins of RGB led to any three of 0,1,2,3,10 & 12 IO ports of the Galileo Gen2 board using jumper wires.
 

3) Now give the command "ls /dev" and check which USB is in use. Suppose, ttyUSB0 is used. Give permission to open the device from the host terminal by typing the command "sudo chmod 777 /dev/ttyUSB0".
 Open putty and give the serial name dev/ttyUSB0 and baud rate 115200. In the putty terminal give login and root to enter.


4) Check whether the IP address of host terminal and serial terminal are in the same network , by giving the command "ifconfig" in both of them.
 If they are not in the same network, configure putty(target) by using the command "ifconfig enp0s20f6 192.168.1.5 netmask 255.255.0.0 up".
 Note: the last digit of the IP address for host should be different from that of the target.


5) To set the target environment ,type "sudo su" in the command terminal and enter the directory where the files are kept using cd command.
To create the executable files, give command "make" in the host terminal to run the Makefile.


6) To copy the executable files from host to target use the command "scp filename root@serialterminal-IP:/home" in the host terminal.


7) In the putty terminal, enter the home directory, using "cd /home" command. Check if the file is copied using "ls" command.


8) To initialize the driver give the command "insmod filename.ko" on putty, then give the command " ./filename PWMinput Red_IOinput Green_IOinput Blue_IOinput".
 Note: give the input arguments corresponding to the IO ports which are connected to the RGB led pins.


9) After the RGB led sequence runs for 4 times, execution is complete and the driver can be released using the command "rmmod filename.ko". 
To release the RGBLed driver mid sequence, press Ctrl+C. Do not press Ctrl+Z.

