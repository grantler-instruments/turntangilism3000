# cityinthecloud-live_captions-signage_player
live captions signage player for rpi with e-ink displays

![display](./doc/display.jpg)
![pihat](./doc/pihat.jpg)
![driver](./doc/driver.jpg)

display size: 800x480



## installation
* `npm install`
* `npm run build`
* start via pm2: `pm2 start dist/index.js --name "signage-player"`
* register as service that starts on boot `pm2 save && pm2 startup`



## usage
* TODO: create PI image with app and scripts installed
* configure .env file: change id, the rest should stay the same, unless you wanna run it against a local directus instance
* restart PI, the app should start automatically
* local dev: `npm run dev`


## dev notes
* epd_7in5_V2_test.py seems to work


## dev installation
* download pi imager: https://www.raspberrypi.com/software/ or via brew `brew install --cask raspberry-pi-imager`
* create an image for your pi model
* activate ssh and wifi connection via the advanced settings
* or do it by hand
  * activate **ssh** by placing a file called ssh in the boot partition
  * create a filed called **wpa_supplicant.conf** in the boot partition
  * adjust with your wifi settings
    ```
    country=DE
    ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
    update_config=1

    network={
        ssid="YourWiFiName"
        psk="YourWiFiPassword"
    }
    ```
* ssh into pi: `ssh user@raspberrypi.local`
* create an ssh key: `ssh-keygen -t ed25519 -C "your_email@example.com"`
* cat `cat ~/.ssh/id_rsa.pub`
* add the public key in the github repo settings under deploy keys
* clone this repo: `git clone git@github.com:threeeeight/cityinthecloud-live_captions-signage_player.git`
* install dependencies: `./scripts/install_dependencies.sh`