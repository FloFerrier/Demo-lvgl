# Demo-LVGL
This project is a UI simulator for Linux.
The goal is to link this UI with an Openpill board (stm32 connected to the computer).
For the first time, we use a script for simulating Openpill board.
## Environment
- VSCode
- CMake
- LVGL
- MariaDB
## Setup
### Startup
```bash
sudo apt install build-essential libsdl2-2.0-0 libsdl2-dev libinput
```
### Create a database with MySQL/MariaDB
#### Add user and database
```bash
$ sudo systemctl start mariadb.service
$ sudo mysql -u root -p
> CREATE DATABASE Openpill
> CREATE USER "openpill-dev"@"localhost" IDENTIFIED BY "<password>";
> USE Openpill;
> GRANT ALL PRIVILEGES on Openpill.* to "openpill-dev"@localhost IDENTIFIED BY "<password>";
> FLUSH PRIVILEGES;
```
#### Use the user and database
```bash
$ mysql -u openpill-dev -p
> SHOW DATABASES;
> USE Openpill;
> SHOW TABLES;
> CREATE TABLE 13July2022
(
id INT AUTO_INCREMENT PRIMARY KEY,
timestamp INT,
temperature INT,
humidity INT,
pressure INT,
eco2 INT
);
> describe 13July2022;
> INSERT INTO 13July2022 (timestamp, temperature, humidity, pressure, eco2) VALUES (0, 22, 48, 998, 440);
> SELECT * FROM 13July2022;
> SELECT temperature FROM 13July2022 ORDER BY id ASC LIMIT 1;
> SELECT temperature FROM 13July2022 ORDER BY id DESC LIMIT 1;
> SELECT temperature FROM 13July2022;
```