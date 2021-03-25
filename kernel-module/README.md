## Flagchecker LKM

### Installation

1. Update to the latest kernel and reboot.
```bash
$ apt-get -y full-upgrade
$ reboot
```

2. Install required dependencies.
```bash
$ apt-get install -y gcc make linux-headers-$(uname -r) python3 python3-requests python3-pip
```

3. Edit files appropriately
4. Run `./build.sh` on the host machine.
5. Don't forget to move your flag generator.
```bash
$ mv generate_flag.py /root/generate_flag.py
```

### Warning

Please use the stable kernel with a bit lower versions. (Currently tested on Ubuntu 18.04 and lower)

Read the main README.md for known issues.
