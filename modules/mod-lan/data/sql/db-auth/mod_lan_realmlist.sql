-- mod-lan: the realm is reached on the LAN address of the server
UPDATE `realmlist` SET `name` = 'LAN Party', `address` = '192.168.2.34', `localAddress` = '192.168.2.34', `localSubnetMask` = '255.255.255.0' WHERE `id` = 1;
