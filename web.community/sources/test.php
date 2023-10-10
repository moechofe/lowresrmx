<?php

include __DIR__.'/redis.php';

$client=new Client('tcp://127.0.0.1:6379');

echo $client->set('ga','bu');
echo PHP_EOL;
echo $client->get('ga');
echo PHP_EOL;

echo $client->del('ga');
echo PHP_EOL;

echo $client->hset('ga','bu','zo');
echo PHP_EOL;
echo $client->hget('ga','bu');
echo PHP_EOL;
