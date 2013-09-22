<?php

$json = array(
		'action' => 'login',
		'name' => 'test1',
		'password' => '123456',
		'id' => 1
);

echo json_encode($json);
echo "\n\n";

$json = array(
		'action' => 'login',
		'name' => 'test2',
		'password' => '123456',
		'id' => 2
);

echo json_encode($json);
echo "\n\n";

$json = array(
		'action' => 'message',
		'token' => 'abcdefg',
		'toid' => 2,
		'id' => 1,
		'content' => '你好test2'
);

echo json_encode($json);
echo "\n\n";

$json = array(
		'action' => 'message',
		'token' => 'abcdefg',
		'toid' => 1,
		'id' => 2,
		'content' => '你好test1'
);

echo json_encode($json);
echo "\n\n";