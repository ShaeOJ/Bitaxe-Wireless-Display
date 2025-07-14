# Bitaxe-Wireless-Display
A Larger Cheap Yellow Display - Display Stats for Your Bitaxe Wirelessly

Choose your way below...Download the File Named Bitaxe_Wireless_Display_bin.zip

⚡ esptool-js User Guide: Flash Your ESP32 in Your Browser! ⚡
Hey there, fellow maker! 👋 Looking for a super convenient way to flash your ESP32 without installing any desktop software? esptool-js is your answer! This guide will walk you through flashing your .bin files directly from your web browser. Get ready for some browser-based magic! ✨

(This guide works across various operating systems, as esptool-js runs in your web browser.)

1. 🌐 Open esptool-js in Your Browser
First things first, let's get the tool open!

Navigate to the esptool-js page: Open your web browser and go to the official esptool-js GitHub Pages site.

Here's the direct link: https://espressif.github.io/esptool-js/

2. 📁 Prepare Your Files & Addresses
You've got your .bin files, right? They're the brains of your operation! Here’s what you need to know about them and where they belong on your ESP32's memory:

Code.ino.bootloader.bin: This little guy needs to go to Address 0x1000

Code.ino.partitions.bin: This one finds its home at Address 0x8000

Code.ino.bin: The main event! This goes to Address 0x10000

Keep these addresses handy – they're super important! 💡

3. 🔌 Hook Up Your ESP32 Board
Time to connect your board to your computer!

Plug it in! Use a trusty USB cable to connect your ESP32 development board.

Drivers check! Make sure you have the necessary USB-to-Serial drivers installed (like CP210x or CH340G, depending on your board's chip). If you've used Arduino IDE with this board before, chances are they're already chilling on your system. If not, a quick search for "CP210x driver" or "CH340G driver" will sort you out! 🛠️

4. ✨ Flashing with esptool-js
Alright, the moment of truth! Follow these steps carefully to flash your board:

🚀 Connect to your ESP32:

In the esptool-js web interface, you'll see a "Connect" button. Click it!

A pop-up window will appear asking you to select a COM port. Choose the one corresponding to your ESP32 board (it might be labeled something like "USB-SERIAL CH340" or "CP210x USB to UART Bridge").

Click "Connect" in the pop-up. If successful, the "Connect" button will change to "Disconnect," and you'll see some initial log messages.

📂 Add the .bin Files and Set Addresses:

Scroll down to the "Flash Settings" section.

For each of your .bin files:

Click the "Add File" button.

A file browser will open. Navigate to the directory where your .bin files are located:
E:\Bitaxe Project\Bitaxe Monitor 1.0\CYD-External-Display-for-Bitaxe-main\CYD-External-Display-for-Bitaxe-main\Code\build\esp32.esp32.jczn_2432s028r\

Select the corresponding .bin file.

Once the file is loaded, you'll see its name and a text box next to it. In that text box, enter the correct memory address we talked about earlier:

For Code.ino.bootloader.bin, enter 0x1000

For Code.ino.partitions.bin, enter 0x8000

For Code.ino.bin, enter 0x10000

Your setup should look something like this in the "Flash Settings" section:

File

Offset

Code.ino.bootloader.bin

0x1000

Code.ino.partitions.bin

0x8000

Code.ino.bin

0x10000

⚙️ Configure Flash Settings:

Below the file list, you'll find other settings. Ensure these are set correctly:

Baud rate: Select 921600 (a good balance of speed and reliability).

Flash mode: DIO (or QIO if you're sure your board supports it and is configured for it).

Flash size: Select the flash size of your ESP32 module (e.g., 4MB, 8MB, 16MB). If unsure, 4MB is a common default.

▶️ Start Flashing:

Once all files are added with their correct offsets and settings are configured, click the "Program" button at the bottom of the "Flash Settings" section.

The tool will attempt to connect to your ESP32 and begin flashing. You might need to press and hold the BOOT button (sometimes labeled IO0 or FLASH) on your ESP32 board while the tool tries to connect, and then release it once the flashing process begins. Some boards enter download mode automatically.

You will see progress messages and a log in the console area below.

Once completed, you should see a "Done" message in the logs. 🎉

🔄 Reset the Board:

You're almost there! After the flashing is complete, give your ESP32 a fresh start by pressing the EN (Enable) or RST (Reset) button on the board. This will restart it and get your newly flashed firmware running!

That's it! Your ESP32 board should now be happily running the code from your .bin files, all thanks to the power of esptool-js in your browser! 🚀

⚡ Espressif Download Tool User Guide: Flash Your ESP32! ⚡
Hey there, fellow maker! 👋 Ready to breathe life into your ESP32 board? This guide will walk you through flashing your .bin files using the super user-friendly Espressif Download Tool. Get ready to make some magic happen! ✨

(This guide is crafted for Windows users, as the tool shines brightest on that platform.)

1. ⬇️ Grab the Espressif Download Tool
First things first, let's get the tool itself!

Head over to the official Espressif website: You'll usually find it lurking in their "Tools" section or by searching for "ESP Flash Download Tool."

Here's a direct link to get you started: https://www.espressif.com/en/support/download/other-tools

Download the latest stable version: Look for something like ESP_Flash_Download_Tool_vX.Y.Z.zip. Always go for the freshest! 🌟

Unzip it! Extract all the goodies from the downloaded ZIP file to a spot that's easy to remember, like C:\Espressif\Flash_Download_Tool.

2. 📁 Prep Your Files & Addresses
You've got your .bin files, right? They're like the brains of your operation! Here’s what you need to know about them and where they belong on your ESP32's memory:

Code.ino.bootloader.bin: This little guy needs to go to Address 0x1000

Code.ino.partitions.bin: This one finds its home at Address 0x8000

Code.ino.bin: The main event! This goes to Address 0x10000

Keep these addresses handy – they're super important! 💡

3. 🔌 Hook Up Your ESP32 Board
Time to connect your board to your computer!

Plug it in! Use a trusty USB cable to connect your ESP32 development board.

Drivers check! Make sure you have the necessary USB-to-Serial drivers installed (like CP210x or CH340G, depending on your board's chip). If you've used Arduino IDE with this board before, chances are they're already chilling on your system. If not, a quick search for "CP210x driver" or "CH340G driver" will sort you out! 🛠️

4. ✨ Flashing with the Espressif Download Tool
Alright, the moment of truth! Follow these steps carefully to flash your board:

🚀 Launch the Tool: Navigate to your extracted folder (e.g., C:\Espressif\Flash_Download_Tool) and double-click the executable (e.g., flash_download_tool_X.Y.Z.exe).

🧠 Select Chip Type:

In the main window, find the "ChipType" dropdown and select ESP32.

Give that "OK" button a friendly click. ✅

📂 Load the .bin Files and Set Addresses:

In the tool, you'll spot a section with rows for "Download Path." This is where the magic happens!

For each of your .bin files:

Check the checkbox next to an empty row.

Click the ... button (the browse button) next to the "Download Path" field.

Navigate to your .bin files' directory:
E:\Bitaxe Project\Bitaxe Monitor 1.0\CYD-External-Display-for-Bitaxe-main\CYD-External-Display-for-Bitaxe-main\Code\build\esp32.esp32.jczn_2432s028r\

Select the corresponding .bin file.

In the "Offset" column for that row, enter the correct memory address we talked about earlier:

For Code.ino.bootloader.bin, enter 0x1000

For Code.ino.partitions.bin, enter 0x8000

For Code.ino.bin, enter 0x10000

Your setup should look something like this (your file paths will be unique, but the addresses are the stars!):

Ch

Download Path

Off

✅

E:\Bitaxe Project\Bitaxe Monitor 1.0\CYD-External-Display-for-Bitaxe-main\CYD-Ext...

0x1000

✅

E:\Bitaxe Project\Bitaxe Monitor 1.0\CYD-External-Display-for-Bitaxe-main\CYD-Ext...

0x8000

✅

E:\Bitaxe Project\Bitaxe Monitor 1.0\CYD-External-Display-for-Bitaxe-main\CYD-Ext...

0x10000

⚙️ Configure SPI Settings:

On the right side of the tool, dial in these settings:

SPI SPEED: Go for 80MHz (or 40MHz if things get a bit finicky).

SPI MODE: Stick with DIO (it's generally the safest bet, even if your board supports QIO).

FLASH SIZE: Pick the flash size of your ESP32 module (e.g., 4MB, 8MB, 16MB). If you're scratching your head, 4MB is a common default for many ESP32 dev boards.

📍 Select COM Port and Baud Rate:

Look to the bottom-left under "COM." Select the COM port that matches your connected ESP32 board. Can't find it? Try unplugging and replugging your board, or peek into your Device Manager under "Ports (COM & LPT)."

Set the BAUD rate to a speedy yet reliable 921600. 🏎️

▶️ Start Flashing:

Hit that glorious START button at the bottom right!

The tool will try to connect. This is where you might need to play a little game with your ESP32: Press and hold the BOOT button (sometimes labeled IO0 or FLASH) on your board while the tool attempts to connect. Release it once the flashing process kicks off! Some clever boards enter download mode automatically.

Watch the progress bar and status messages in the "STATUS" window. It's like watching a digital masterpiece being created! 🎨

When it's done, you'll see a triumphant "FINISH" message! 🎉

🔄 Reset the Board:

You're almost there! After the flashing is complete, give your ESP32 a fresh start by pressing the EN (Enable) or RST (Reset) button on the board. This will restart it and get your newly flashed firmware running!

That's it! Your ESP32 board should now be happily running the code from your .bin files. Go forth and create amazing things! 🚀
