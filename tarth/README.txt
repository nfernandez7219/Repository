How to compile test software application
    1.) rdma_rsp: responder side
        go to the path where the ../sw/rsp
        type "make"
    2.) rdma_req: requester side
        go to the path where the ../sw/rsp
        type "make"



How to compile and run firmware via vitis
    1.) Compile: find/click the hummer symbotl on the upper left of the vitis window.
    2.) run: find/click the "bug" symbolt on the upper left of the vitis window.



How to prepare an FPGA to be a mellanox NIC
    1.) Reboot the system where the FPGA is installed
    2.) Remove PCI Device via terminal using “echo 1 > /sys/bus/pci/devices/0000:05:00.0/remove”
    3.) Load FPGA image “”.bit””, via terminal
        Go where the .bit file was saved
        Source /opt/Xilinx/Vivado/2021.1/settings64.sh
        Xsdb
        Connect
        fpga rsnic_app.bit
    4.) Reboot the system again
    5.) Upon boot-up, check pci devices via terminal “”lspci | grep Eth””
        It will show “”Ethernet controller: Mellanox Technologies

    6.) Remove pci device via terminal using “echo 1 > /sys/bus/pci/devices/0000:05:00.0/remove
    7.) Load and run the firmware using vitis IDE
    8.) Load mellanox driver via terminal “echo 1 > /sys/bus/pci/rescan”
    9.) Check if the driver was loaded via terminal “dmesg -c”



How to run test application
    1.) Open two terminal, one for RSP testp app, one for REQ test app
    2.) Run the RSP test app via terminal, “./rdma_rsp”
        usage: ./rdma_req
        [-s server_address]
        [-p port_number]
    3.) Run the REQ test app via terminal, “./rdma_req”
        usage: ./rdma_req
        [-s server_address]
        [-p port_number]
    4.) follow the next step shows on the test app terminal
    

