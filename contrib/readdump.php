<?php

error_reporting(E_ALL & ~E_NOTICE);

$f = $_SERVER['argv'][1];

if (!$f) die('Usage: '.$_SERVER['argv'][0].' file'."\n");

$fp = fopen($f, 'r');
if (!$fp) die("failed to open file $f\n");
$data_version = 0;

// read file
while(!feof($fp)) {
	$lb = fread($fp, 8);
	if (strlen($lb) == 0) {
		break; // EOF
	}
	if (strlen($lb) != 8) {
		die("failed to read data, got incomplete frame\n");
	}

	list(,$l) = unpack('q', $lb);

	if ($l > 10*1024*1024) { // 10MB
		die("packet too large at 0x".dechex(ftell($fp)-8).": $l ".bin2hex($lb)."\n");
	}

	$next_pos = ftell($fp) + $l;

	$type = ord(fread($fp, 1));
	list(,$time) = unpack('q', fread($fp, 8));
	list(,$cnx_id) = unpack('q', fread($fp, 8));

	$time_str = date('Y-m-d H:i:s', $time/1000);
	$prefix = $time_str.' #'.$cnx_id.': ';


	switch($type) {
		case 1: // request
			$method = stream_get_line($fp, 4096, "\0");
			$url = stream_get_line($fp, 4096, "\0");
			list(, $header_count) = unpack('l', fread($fp, 4));
			echo $prefix.'Request: '.$method.' '.$url." - $header_count headers\n";
			break;
		case 2:
			echo $prefix.'Data ('.($l - 17).' bytes)'."\n";
			break;
		case 3:
			list(, $http_code) = unpack('l', fread($fp, 4));
			$http_resp = stream_get_line($fp, 4096, "\0");
			list(, $header_count) = unpack('l', fread($fp, 4));
			echo $prefix.'Response headers - HTTP '.$http_code.' '.$http_resp." - $header_count headers\n";
			break;
		case 4:
			echo $prefix.'EOF'."\n";
			break;
		case 255:
			// version packet
			list(,$version) = unpack('q', fread($fp, 8));
			echo $prefix.'Announced data version 0x'.dechex($version)."\n";
			$data_version = $version; // upgrade version number for future parsing
			break;
		default:
			echo $prefix.'Packet type '.$type."\n";
	}

	fseek($fp, $next_pos);
}
