<?php

class ErrorRespond extends Exception
{
}

class ErrorMoved extends Exception
{
	private $slot = 0;
	function getSlot() { return $this->slot; }
	function __construct($slot,$msg="",$code=0,$prev=null)
	{
		assert('is_numeric($slot) and $slot>=0 and $slot<=16384');
		$this->slot = (int)$slot;
		parent::__construct($msg,$code,$prev);
	}
}

class ErrorCrosslot extends Exception
{
}

class ErrorConnection extends Exception
{
}

class ErrorProtocol extends Exception
{
}

class Intel
{
	private $cluster = null;

	function __construct(array $dsn)
	{
		assert('array_filter($dsn,\'is_scalar\')===$dsn');

		$this->cluster = new Cluster($dsn);
	}

	private function extractSubKey($key)
	{
		assert('is_string($key)');

		$a = strpos($key,'{');
		if($a === false) return $this->cluster[$key];
		
		$b = strpos($key,'}');
		if(!($b > $a+1)) return $this->cluster[$key];

		return substr($key,$a+1,$b-$a-1);
	}

	function clientByKey($key)
	{
		return $this->cluster[$this->extractSubKey($key)];
	}

    function __call($cmd, array $args)
	{
		switch(strtolower($cmd))
		{
		// Get the key from the first arg
		// Intel::CMD(KEY1,?..)
		case 'append':
		case 'bitcount':
		case 'bitpos':
		case 'blpop':
		case 'brpop':
		case 'brpoplpush':
		case 'decr':
		case 'decrby':
		case 'del':
		case 'dump':
		case 'exists':
		case 'expire':
		case 'expireat':	
		case 'get':
		case 'getbit':
		case 'getrange':
		case 'getset':
		case 'hget':
		case 'hincrby':
		case 'hincrbyfloat':
		case 'hkeys':
		case 'hlen':
		case 'hmget':
		case 'hmset':
		case 'hset':
		case 'hsetnx':
		case 'hstrlen':
		case 'hvals':
		case 'incr':
		case 'incrby':
		case 'incryfloat':
		case 'mget':
		case 'mset':
		case 'msetnx':
		case 'lindex':
		case 'linsert':
		case 'llen':
		case 'lpop':
		case 'lpush':
		case 'lpushx':
		case 'lrange':
		case 'lren':
		case 'lset':
		case 'ltrim':
		case 'mget':
		case 'move':
		case 'mset':
		case 'msetnx':
		case 'persist':
		case 'pexpire':
		case 'pexpireat':
		case 'pfadd':
		case 'pfcount':
		case 'pfmerge':
		case 'psetex':
		case 'pttl':
		case 'rename':
		case 'renamenx':
		case 'restore':
		case 'rpop':
		case 'rpoplpush':
		case 'rpush':
		case 'rpushx':
		case 'sadd':
		case 'scard':
		case 'sdiff':
		case 'sdiffstore':
		case 'set':
		case 'psetex':
		case 'setbit':
		case 'setex':
		case 'setnx':
		case 'setrange':
		case 'strlen':
			$client = $this->clientByKey($args[0]);
			break;

		// Get the key from the second arg
		// Intel::CMD(?,KEY1,..)
		case 'bitop':
			$client = $this->clientByKey($args[1]);
			break;

		// Get the key from the third arg
		// Intel::CMD(?,?,KEY1,..)	
		case 'eval':
		case 'evalsha':
		case 'migrate':
			$client = $this->clientByKey($args[2]);
			break;

		default:
			// TODO: create an exception
			throw new BadMethodCallException;
		}

		try
		{
			$r = call_user_func_array(array($client,$cmd),$args);
		}
		catch(ErrorMoved $e)
		{
			$r = call_user_func_array(array($this->cluster->clientBySlot($e->getSlot()),$cmd),$args);
		}

	}

}

class Cluster implements ArrayAccess
{
    static private $crc16_redis_table = array(
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0,
    );
    
    static private function crc16_redis($val)
	{
		assert('is_string($val)');
        $table = self::$crc16_redis_table;
        $crc = 0;
        for($i=0,$l=strlen($val); $i<$l; $i++)
            $crc = (($crc << 8) ^ $table[($crc >> 8) ^ ord($val[$i])]) & 0xFFFF;
        return $crc;
    }

	private $dsn = array();
	function __construct(array $dsn)
	{
		assert('array_filter($dsn,\'is_scalar\')===$dsn');

		$this->dsn = $dsn;
		$this->num = count($dsn);
	}

	private $client = array();

	function clientByIndex($idx)
	{
		assert('isset($this->dsn[$idx])');

		return $this->clientByDSN($this->dsn[$idx]);
	}

	function clientByDSN($dsn)
	{
		assert('is_string($dsn) and $dsn');

		if(!isset($this->client[$dsn]))
		{
			$this->client[$dsn] = new Client($dsn);
		}

		return $this->client[$dsn];
	}

	function clientBySlot($slot)
	{
		assert('is_int($slot) and $slot>=0 and $slot<=16384');

        $index = min((int)($slot / (int)(16384/$this->num)), $this->num - 1);

		return $this->clientByIndex($index);
	}

	function offsetExists($subkey)
	{
		assert('is_string($subkey)');

		return (bool)$this->offsetGet($subkey);
	}

	function offsetGet($subkey)
	{
		assert('is_string($subkey)');

		$slot = self::crc16_redis($subkey) & 0x3FFF;

		return $this->clientBySlot($slot);
	}

	function offsetSet($subkey, $no)
	{
		throw new BadMethodCallException("Nothing to do with ArrayAccess: ".__CLASS__.":".__FUNCTION__.'()');
	}

	function offsetUnset($subkey)
	{
		throw new BadMethodCallException("Nothing to do with ArrayAccess: ".__CLASS__.":".__FUNCTION__.'()');
	}


    function __call($cmd, array $args)
	{
		$index = mt_rand(0, $this->num-1);
		try
		{
			return call_user_func_array(array($this->clientByIndex($index),$cmd),$args);
		}
		catch(ErrorMoved $e)
		{
			return call_user_func_array(array($this->clientByDSN($e->getDSN()),$cmd),$args);
		}
	}

}

class Client
{
    private $reader = null;
    private $socket = null;

	function __construct($dsn)
    {
		assert('is_string($dsn)');

        $this->reader = phpiredis_reader_create();
        phpiredis_reader_set_status_handler($this->reader, function($status)
        {
            switch($status)
            {
                case 'OK': return true;
                case 'QUEUED': return true;
                default: return $status;
            }
        });
        phpiredis_reader_set_error_handler($this->reader, function($msg)
        {
			if(substr($msg,0,6)=='MOVED ')
			{
				$begin = strpos($msg,' ')+1;
				$len = strrpos($msg,' ') - $begin;
				throw new ErrorMoved(substr($msg,$begin,$len));
			}
			else
            	throw new ErrorRespond($msg);
        });

        $this->socket = @stream_socket_client($dsn, $errno, $errstr, 2, STREAM_CLIENT_CONNECT);
        if(!$this->socket)
        {
            throw new ErrorConnection($errstr, $errno);
        }

        //stream_set_timeout($this->socket, 3);
    }

    function __destruct()
    {
        fclose($this->socket);
        phpiredis_reader_destroy($this->reader);
    }

    function __call($cmd, array $args)
    {
        assert('is_string($cmd)');
        assert('array_filter($args,\'is_scalar\')===$args');

        array_unshift($args, $cmd);
        $buffer = phpiredis_format_command($args);

        // fwrite with retry
        // See: http://fr2.php.net/manual/fr/function.fwrite.php
        $bytes_to_write = strlen($buffer);
        $bytes_written = 0;
        while($bytes_written < $bytes_to_write)
        {
            if($bytes_written==0) $rv = fwrite($this->socket, $buffer);
            else $rv = fwrite($this->socket, substr($buffer, $bytes_written));
            if($rv===false || $rv==0) break;
            $bytes_written += $rv;
        }
		
        while(PHPIREDIS_READER_STATE_INCOMPLETE===($state=phpiredis_reader_get_state($this->reader)))
        {
            $buffer = fread($this->socket, 4096);
            if($buffer===false || $buffer==='') throw new ErrorConnection;
            phpiredis_reader_feed($this->reader, $buffer);
        }

        if($state===PHPIREDIS_READER_STATE_COMPLETE) return phpiredis_reader_get_reply($this->reader);
        else throw new ErrorProtocol(phpiredis_reader_get_error($this->reader));
    }
}

