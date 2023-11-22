Introduction
============

The DVB ASI Receive is a PCI computer card with a BNC type connector, which 
will accept DVB Asynchronous Serial Interface (ASI) standard inputs.
 

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
	do it later from the Receiver application.
4.  After installation program finished the system must be rebooted to complete
    the DVB ASI card installation.


Setup
=====

The DVB ASI Receive card have two types of setup:

1. "DVB ASI card system driver setup" (file DvbCfg.exe).
   -------------------------------------------------------
   DvbCfg.exe is a setup program provided by the card manufacturer. This setup allows
   you to change the system driver parameters for DVB ASI card. To apply the changes
   made in the dialog, the system should be rebooted.

   Under normal conditions this setup should be used only during the installation
   to define DVB ASI device name and to configure driver buffer size. Remaining
   parameters can be configured from MainData Receiver setup.

   Setup parameters:
		- DVB Device Name - This is where the name of the Dvb ASI card is
		  entered, you may enter as many devices as you have actual cards
		  installed in your machine. If you have more devices entered than
		  actual cards then the DVB ASI will not work.
		- <--Add Device - This parameter is the next available device that
		  may be entered 
		- Stuffing between bytes - This box is where the IB stuffing is
		  entered, (HEX) (what stuffing is expleined in section "Data
		  Transmission Using DVB")
		- Stuffing between frames - This is where the IP stuffing is
		  entered, (HEX)
		- <-Specify - Click here after the stuffing parameters have been
		  entered to enter them into the registry.
		- Rate (bit/sec) - Enter the rate that you want the DVB ASI
		  Transmit card to transmit at.
		- Specify-Click here after the bit rate has been entered in the Rate
		  box to enter it into the registry.
		- % error - probability of error durring transmission
		- No IB - chceck this for no Inter-Byte stuffin(=0)
	        - Rx - Max Buffer Size - Enter the size of each of the receive buffers
		  here default is set to 0x100000.
	        - Rx - Max Buffer Numbers - Enter the number of receive buffers wanted
		  here default is set to 2. (don't use lower than 2!)
		- Tx - Max Buffer Size - Enter the size of each of the transmit
		  buffers here default is set to 0x100000.
		- Tx - Max Number of Buffers - Enter the number of transmit buffers
		  wanted here default is set to 2. ( don't use lower than 2 ! )
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
    Run the DVB Receiver application and select from menu Setup/Receiver Card (Edit).
    This setup allows you to change basic parameters for receiving via DVB ASI card.

    Setup parameters:
		- Packet sync - Enable Transport Stream packet synchronization for 
		  receive
		- RF High -  HotLink RL pin set HIGH during normal receive operation
		  (otherwise it's LOW)
   

Uninstallation
==============

DVB ASI card will be automatically uninstalled with our DVB Receiver software product.


Card settings
=============

File: <DVB directory>\config\DvbClient.cfg (Receiver configuration file)

In the NULL section specify:
	connectString = DvbAsi

Further settings are described below. However, be carefull when changing them.
We suggest to use default values as shown in the table.
This settings is used and can be changed also in our application menu item 
Setup/Receiver Card (Edit).

Setting				default			comment
--------------------------------------------------------------------------------------------
[DvbAsiConfig]
   Speed 			4.000000		* only for DVB ASI send card
   204 bytes frames  		0		* only for DVB ASI send card
   Packet sync  		1			* 
   RF High  			1			* 
   No Inter-Byte Stuffing  	0		* only for DVB ASI send card



Software Specifications:
========================
DVB-KNT4			NT 4 Kernel Mode DVB Transmit and Receive Driver


Data Transmission Using DVB
===========================

When the receiver device is opened, the PCI bridge is programmed and receiving
will begin when the card recognizes a DvB sync character or characters in the
input stream. These DvB sync characters are specified in the DvB protocol
and seem to be used solely to recover byte synchronization. 
