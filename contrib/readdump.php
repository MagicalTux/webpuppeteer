<?php

error_reporting(E_ALL & ~E_NOTICE);
require_once(__DIR__.'/webpuppeteer.class.php');

$f = $_SERVER['argv'][1];

if (!$f) die('Usage: '.$_SERVER['argv'][0].' file'."\n");

$obj = new WebPuppeteer($f);

$count = $obj->getLastPacket();
for($i = 1; $i <= $count; $i++) {
	$pkt = $obj->getPacketData($i);
	$prefix = date('Y-m-d H:i:s', $pkt['time']/1000).' #'.$pkt['id'].': ';
	switch($pkt['type']) {
		case WebPuppeteer::TYPE_REQ:
			echo $prefix.'Request: '.$pkt['method'].' '.$pkt['url'].' - '.json_encode($pkt['headers'])."\n";
			break;
		case WebPuppeteer::TYPE_DATA:
			echo $prefix.'Data ('.$pkt['body'].' bytes)'."\n";
			break;
		case WebPuppeteer::TYPE_RES:
			echo $prefix.'Response headers - HTTP '.$pkt['http_code'].' '.$pkt['http_resp'].' - '.json_encode($pkt['headers'])."\n";
			break;
		case WebPuppeteer::TYPE_EOF:
			echo $prefix.'EOF'."\n";
			break;
		case WebPuppeteer::TYPE_VERSION:
			echo $prefix.'Announced data version 0x'.dechex($pkt['version'])."\n";
			break;
		default:
			echo $prefix.'Unknown'."\n";
	}
}

