<?php

include __DIR__.'/redis.php';

$client=new Client('tcp://127.0.0.1:6379');

include __DIR__.'/index.html';
