                       MAIN DATA DVB SERVER

       Instructions for the configuration of "ISA RS-422, Main Data" card.

Introduction
------------

The Card is controlled by Windows NT Kernel Mode Driver. The Card/Driver communication
relies (as usual for ISA boards) on the interrupt mechanism.

Interrupt is characterized by two values: IRQ and i/o address. These parameters must be:
- Selected so that they do not conflict with other ISA devices installed on the system.
- Set physically on the Card via hardware jumpers.
- Specified in the Driver settings.

Selection of the proper interrupt values is perhaps the most complicated step in the
Card installation. The remaining installation and the Card configuration is relatively
straightforward process. 
Once the Card is installed, it does not require any maintenance.


Installation
-------------

Installation of the ISA Card consists of these steps:

- Selection of the interrupt parameters as described under "Hardware configuration".

- Physical installation of the Card:
  The Card jumpers must be switched to represent selected interrupt values and the Card
  must be inserted into a free ISA slot. (See also "Hardware configuration".)

- Installation of the Card driver (md_DvbDriver.sys).
  This step is done via Main Data Installation program. The user running the installation
  must have administrator privileges.
  The installation copies md_DvbDriver.sys into system directory and creates Driver entry
  in the Registry:
        HKLM\SYSTEM\CurrentControlSet\Services\md_DvbDriver
  The Install program also makes sure that the software settings described under
  "Card settings" are set to correct values and all you have to do is to select suitable
  IRQ and port.

- Setting up Card/Driver communication.
  This basically denotes telling the Driver which interrupt parameters have to be used.
  The parameters (the same as switched physically on the Card!) can be edited:
  -- via Driver configuration dialog (accessible from the Installation program or from
     the Application Setup menu),
  -- manually by changing Driver key in the Registry (via RegEdit.exe).
  After the parameters are changed the Driver must be restarted - either using Control Panel/
  Devices or simply via system restart.

- Setting up software settings. These settings determine the size of the Driver buffer,
  packet format etc. and are described under "Card settings". As a rule, this step can
  be omitted as the defaults setup by Install program should be sufficient.


Hardware configuration
-----------------------

IRQ and port address must be set manually on the card as follows:

Jumper 1
   5 values meaning following IRQ's (left to right):
       5, 9, 10, 11, 12

Jumper 2
   5 DIP switches denoting binary address 0x200 + 16*adr, 1 is the least significant bit

   E.g. 0x280 is switched as:
       1 2 3 4 5
       - - - + -

When planning IRQ/port resources keep in mind following:
- You must select values which are not occupied by another device.
- Make sure that BIOS computer setup enables the use of particular IRQ.
- The Card needs 16 consecutive Bytes of free address space.

Windows NT:
To see which values are free in the system, run Start menu utility
    Programs / Administrative tools / Windows NT Diagnostics
and switch to Resources tab (pages IRQ, resp. I/O Port).


Software configuration
----------------------

Run the DVB Server application and open menu item Setup/Sender Card (Edit).
Specify here IRQ and I/O address as set under Hardware configuration.
To test if the configuration is working, press the "Try Card Handshake" button.
This step is important as it restarts the card driver dynamically.

If the dynamic restart does not work, you have to restart the card driver by force.
On Windows NT (provided you have administrative privileges) the driver can be restarted
from the Control Panel/Devices (the driver name is md_DvbDriver).
If this procedure does not work you still can reboot the machine.


Card settings
--------------

File: <DVB directory>\config\Dvb.cfg (Server configuration file)

In the NULL section specify:
	connectString = DVB

Further settings are described bellow. However, be carefull when changing them.
We suggest to use default values as shown in the table.


Setting		  default	  comment
------------------------------------------------------

[CardState]
Flags           = 0x44	*
Speed 			= 10000	* [kHz], frequency of the Card (10000..16000); only if Card generates clock

[DriverState]
DriverMemory    = 200	* queue size for the DVB driver (# of packets),
Flags           = 0		* 1-dump packet contents, 2-dump packet id's


"CardState.Flags" value is hexadecimal number consisting of following bits:
   0xnn, where
	0x02	1 - the card will cut TS Header
	0x04	1 - Card will generate MPEG2 NULL packets in case no data for send are available
	0x40	Clock source: 0 - internal (i.e. card), 1 - external (e.g. multiplexor)
	0x80	Data valid edge: 0 - raise(0->1), 1 - fall(1->0)
