Introduction
============

The DVB ASI Send is a PCI computer card with a BNC type connector, which
is capable of transmitting a DVB Asynchronous Serial Interface (ASI) standard
stream.  


Installation of LS DVB ASI card
===============================

1.  Insert the DVB ASI Send card into the first available PCI slot, boot the computer.
2.  Run Installation of Main Data Server.
    When the installation program will ask for type of your DVB card, choose
    "DVB ASI"
3.  The installation program runs the "DVB ASI system driver setup" (see below).
    If there is nothing specified in the DVB Device Name box press "Add"
    button and the first Device Name that is available will be added,
    then press "Apply" to accept the parameters.
	As part of the installation you can also configure the Card, but you can also
	do it later from the Server application.
4.  After installation program finished the system must be rebooted to complete
    the DVB ASI card installation.


Setup
=====

The DVB ASI Send card has two types of setup:

1. "DVB ASI card system driver setup" (file DvbCfg.exe).
   -------------------------------------------------------
   DvbCfg.exe is a setup program provided by the card manufacturer. This setup allows
   you to change the system driver parameters for DVB ASI card. To apply the changes
   made in the dialog, the system should be rebooted.

   Under normal conditions this setup should be used only during the installation
   to define DVB ASI device name and to configure driver buffer size. Remaining
   parameters can be configured from MainData Server setup.

   Setup parameters:
		- DVB Device Name - This is where the name of the Dvb ASI card is
		  entered. Create one device for each DVB ASI card installed in 
		  your machine.
		- <--Add Device - This parameter is the next available device that
		  may be entered 
		- Stuffing between bytes - This box is where the IB stuffing is
		  entered, (HEX) (Stuffing is explained in section "Data
		  Transmission Using DVB".)
		- Stuffing between frames - This is where the IP stuffing is
		  entered, (HEX)
		- <-Specify - Click here after the stuffing parameters have been
		  entered to enter them into the registry.
		- Rate (bit/sec) - Enter the rate that you want the DVB ASI
		  Transmit card to transmit at.
		- Specify-Click here after the bit rate has been entered in the Rate
		  box to enter it into the registry.
		- % error - probability of error during transmission
		- No IB - check this for no Inter-Byte stuffing(=0)
	        - Rx - Max Buffer Size - Enter the size of each of the receive buffers
		  here default is set to 0x100000.
	        - Rx - Max Buffer Numbers - Enter the number of receive buffers wanted
		  here default is set to 2. (don't use lower than 2!) 
		- Tx - Max Buffer Size - Enter the size of each of the transmit
		  buffers here default is set to 0x100000.
		- Tx - Max Number of Buffers - Enter the number of transmit buffers
		  wanted here default is set to 2. (don't use lower than 2 !)
		- 204 Bytes frames - Check this box if 204 byte packets are wanted.
		  Leave unchecked for default 188-byte packet size.
		- RL Always High - Reframe Link (not used in normal operation)
		- Enable Packet Sync - Enable Transport Stream packet synchronization
		  for receive
		- Apply - Will accept all parameters entered and will close the applet
		- Add - Will add the next device that is available
		- Delete - Will delete the device that is highlighted in the Device 
		  Name box
		- Cancel - Will cancel the installation process
		- Help - Will bring up the DvbCfg help box

2.  "DVB ASI card application setup" 
    --------------------------------
    Run the DVB Server application and select from menu Setup/Sender Card (Edit).
    This setup allows you to change basic parameters for transmission via DVB ASI card.
    
	Setup parameters:
		- Transmission speed - Speed of Sending data in Mbit/sec
		- Read Solomon - Transmit interframe stuffing generated for 204 byte
		  packet (otherwise 188 byte)
		- No IB stuffing - check this for no Inter-Byte stuffing(=0).


Uninstallation
==============

DVB ASI card will be uninstalled automatically together with the DVB Server.


Card settings
=============
File: <DVB directory>\config\Dvb.cfg (Server configuration file)

In the NULL section specify:
	connectString = DvbAsi

Further settings are described below. However, be careful when changing them.
We suggest to use default values as shown in the table.
This settings is used and can be changed also in our application menu item 
Setup/Sender Card (Edit).

Setting				default			comment
--------------------------------------------------------------------------------------------
[DvbAsiConfig]
   Speed 			4.000000		* 
   204 bytes frames		0			* 
   Packet sync 			1			* only for DVB ASI receive card
   RF High 			1				* only for DVB ASI receive card
   No Inter-Byte Stuffing	0		* 



Software Specifications:
========================

Driver DVB-KNT4 supplied with the system runs only on WinNT 4.


Data Transmission Using DVB
===========================

When the driver is opened for transmit, the byte counter is zeroed. As bytes
are transmitted, every byte is interspersed with the number of bytes selected
for interbyte padding, and every 188 or 204 bytes an additional specified
interframe padding is included. This may be required to support equipment
which can only process limited reception bandwidth and requires an idle period
between transport frames.
