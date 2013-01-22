<?php
$address = 'jabbercn.org';
$service_port = '5222';
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket < 0) {
    echo "socket_create() failed: reason: " . socket_strerror($socket) . "\n";exit;
} 

$result = socket_connect($socket, $address, $service_port);
if ($result < 0) {
    echo "socket_connect() failed.\nReason: ($result) " . socket_strerror($result) . "\n";exit;
} 

$in = "<?xml version='1.0'?><stream:stream to='jabbercn.org' version='1.0' xml:lang='en' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>";

$out = '';

socket_write($socket, $in, strlen($in));

echo "Reading response:\n\n";
while ($out = socket_read($socket, 2048)) {
    echo $out;
}


socket_close($socket);
