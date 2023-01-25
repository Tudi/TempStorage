#!/bin/bash

sudo cp /var/www/html/settings.php /var/www/html/settings.php.backup

sudo cp -R /home/ec2-user/git/* /var/www/html/

sudo rm -rf /var/www/html/scripts

sudo cp /var/www/html/settings.php.backup /var/www/html/settings.php

sudo rm -rf /var/www/html/settings.php.backup
sudo rm -rf /var/www/html/appspec.yml
sudo rm -rf /var/www/html/assume-role.sh
sudo rm -rf /var/www/html/README.md
sudo rm -rf /var/www/html/IsRegisteredJavascript.html
