# Prepare

#### 1) Installation petalinux 2018.3
We recommend using ubuntu 16.04

1. Download PetaLinux 2018.3 Installer
https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools/2018-3.html
2. Install the dependencies
`$ sudo apt-get install -y python gawk gcc git make net-tools libncurses5-dev tftpd zlib1g:i386 libssl-dev flex bison libselinux1 gnupg wget diffstat chrpath socat xterm autoconf libtool tar unzip texinfo zlib1g-dev gcc-multilib build-essential libsdl1.2-dev libglib2.0-dev screen pax gzip`
3. Untar and install petalinux
`$ petalinux-v2018.3-final-installer.run [install path]`
4. Run settings.sh
`$ source [install path]/settings.sh`
This is a required step for assembling petalinux. Repeat only after rebooting the system or in a new terminal

Full installation guide
https://www.instructables.com/id/Getting-Started-With-PetaLinux/

#### 2) Build petalinux
1. Download sources
`$ git clone https://github.com/DEEF137/NUT8NT_Petalinux_system`
2. Change directory
`$ cd nut8_petalinux_2018.3_amungo/`
3. Copy .hdf file to this directory and configure project
`$ petalinux-config --get-hw-description`
4. Build
`$ petalinux-build`
5. Package
`$ cd images/linux`
`$ petalinux-package --boot --fsbl zynqmp_fsbl.elf --fpga system.bit --u-boot --force`

#### 3) Prepare SD card
Full guide
https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842385/How+to+format+SD+card+for+SD+boot

#### 4) Install petalinux on SD card
1. Copy BOOT.BIN and image.ub from nut8_petalinux_2018.3_amungo/images/linux to Boot partition on SD
2. Copy all files from nut8_petalinux_2018.3_amungo/images/linux/rootfs.tar.gz to rootfs partition on SD

Now you can run NUT8NT
login: root
password: root

# About soft
- All soft is in SOFTWARE folder
- **nut8_init** - init all system. Run onec after each reboot
- **NUT8_DMA2UDP** - sends pieces of data on UDP to port 30137 
usage `$ NUT8_DMA2UDP address channel`
address - UDP server address
channel - number of channel inrange [0;7]
UDP_rec.grc - UDP server on GNU-Radio
- **NUT8_DMA2TCP** - send 500MB of data with all channels.
usage `$ NUT8_DMA2TCP address`
- **NUT8_DMA2TCP_5MHz** - send data by TCP.
usage `$ NUT8_DMA2TCP address`
- **NUT8_TCP2Files** - TCP server for recieve dumps from all channels
- **NUT8_File2Channels** - Splits a dump into the data of each channel
usage `$ NUT8_File2Channels -g fileIn fileOut`
- **GNSS_SDR_Class-ComplexAndCNo** - matlab scripts for testing dumps adn fin. In initSettings.m you can change dump file and correlation search

# Build soft
1. **nut8_init, NUT8_DMA2UDP, NUT8_DMA2TCP**
1.1. `$ cd nut8_init/` or `$ cd NUT8_DMA2UDP/` or `$ cd NUT8_DMA2TCP/`
1.2. `$ mkdir build`
1.3. `$ cd build`
1.4. `$ cmake .. -D CMAKE_C_COMPILER=aarch64-linux-gnu-gcc -D CMAKE_CXX_COMPILER=aarch64-linux-gnu-g++`
2. **NUT8_TCP2Files**
2.1. `$ cd NUT8_TCP2Files/` 
2.2. `$ qmake`
2.3. `$ make`

# Install soft
 **nut8_init, NUT8_DMA2UDP, NUT8_DMA2TCP_5MHz** you must send on NUT8NT via FTP or copy on sd in /home/root
 Copy files **ConfigSet_all_GPS_L1_patched_ldvs_noadc.hex** and **Init_NUT8.sh** to /home/root
 
 # Run
 1. Prepare system on NUT8
 `$ Init_NUT8.sh`
 2. Start TCP server on host computer
 `$ NUT8_TCP2Files`
 3. Get data on NUT8
  `$ NUT8_DMA2TCP_5MHz address`
 4. Near NUT8_TCP2Files  on the host computer will be records
 5. Run NUT8_File2Channels 
