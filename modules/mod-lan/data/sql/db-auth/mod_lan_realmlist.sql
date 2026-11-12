-- mod-lan: the realm is reached on the LAN address of the server
UPDATE `realmlist` SET `name` = 'LAN Party', `address` = '192.168.50.14', `localAddress` = '192.168.50.14', `localSubnetMask` = '255.255.255.0' WHERE `id` = 1;
