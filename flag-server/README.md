## Flag checker server.

### WARNING

Make sure that you know what you're trying to do.

Before the production, Make sure to read `index.php` multiple times and fix ones that need to be fixed.

### How to run

```sh
docker-compose down
docker-compose up -d
```

### Note

It is highly recommended to follow the order of the script.

start.sh
```sh
#!/bin/sh

# Security
sysctl -w kernel.dmesg_restrict=1

# Docker cleanup
docker rm -f $(docker ps -a -q)

# Build and run challenges
cd /srv/challenge/example/
docker-compose down && docker-compose up -d

# Install flagchecker-kernel-module
cd /srv/flagchecker/kernel-module/example
./build.sh

# Install flagchecker-server
cd /srv/flagchecker/flag-server/
docker-compose down
docker-compose up -d
```

crontab
```
@reboot /srv/start.sh
```
