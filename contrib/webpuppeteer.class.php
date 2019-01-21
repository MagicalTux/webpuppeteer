<?php

// read only access

class WebPuppeteer {
	private $fp;
	private $index = [];

	const TYPE_REQ = 1;
	const TYPE_DATA = 2;
	const TYPE_RES = 3;
	const TYPE_EOF = 4;
	const TYPE_VERSION = 255;

	public function __construct($file) {
		$this->fp = fopen($file, 'r');
		if (!$this->fp)
			throw new \Exception('Failed to open file');

		$this->buildIndex();
	}

	public function fetchUrlStreams($url, $method = 'GET') {
		return $this->index['url'][$url][$method];
	}

	public function getStreamRequest($stream) {
		return $this->getStreamPacket($stream, self::TYPE_REQ);
	}

	public function getStreamResponse($stream) {
		return $this->getStreamPacket($stream, self::TYPE_RES);
	}

	protected function getStreamPacket($stream, $type) {
		$info = $this->index['stream'][$stream];
		if (!$info) throw new \Exception('Invalid stream provided');

		foreach($info as $p) {
			$pkt = $this->getPacket($p);
			if ($pkt['type'] == $type) return $this->getPacketData($p);
		}

		return false;
	}

	public function getStreamData($stream, $fp) {
		$info = $this->index['stream'][$stream];
		if (!$info) throw new \Exception('Invalid stream provided');

		foreach($info as $p) {
			$pkt = $this->getPacket($p);
			if ($pkt['type'] != self::TYPE_DATA) continue;
			fwrite($fp, $this->getPacketBody($p));
		}
	}

	public function getLastPacket() {
		return $this->index['last_packet'];
	}

	public function getPacket($id) {
		$info = $this->index['pkt'][$id];
		if (!$info) throw new \Exception('Invalid packet id '.$id);

		return $info;
	}

	public function getPacketData($id) {
		$pkt = $this->getPacket($id);
		fseek($this->fp, $pkt['body_offset']);

		switch($pkt['type']) {
			case self::TYPE_REQ:
				$method = stream_get_line($this->fp, 4096, "\0");
				$url = stream_get_line($this->fp, 4096, "\0");
				list(, $header_count) = unpack('l', fread($this->fp, 4));
				$headers = [];
				for($i = 0; $i < $header_count; $i++) {
					$k = stream_get_line($this->fp, 4096, "\0");
					$v = stream_get_line($this->fp, 4096, "\0");
					$headers[] = [$k, $v];
				}

				$pkt['method'] = $method;
				$pkt['url'] = $url;
				$pkt['headers'] = $headers;
				return $pkt;
			case self::TYPE_DATA:
				return $pkt;
			case self::TYPE_RES:
				list(, $http_code) = unpack('l', fread($this->fp, 4));
				$http_resp = stream_get_line($this->fp, 4096, "\0"); 
				list(, $header_count) = unpack('l', fread($this->fp, 4));
				$headers = [];
				for($i = 0; $i < $header_count; $i++) {
					$k = stream_get_line($this->fp, 4096, "\0");
					$v = stream_get_line($this->fp, 4096, "\0");
					$headers[] = [$k, $v];
				}
				$pkt['http_code'] = $http_code;
				$pkt['http_resp'] = $http_resp;
				$pkt['headers'] = $headers;
				return $pkt;
			case self::TYPE_EOF:
				return $pkt;
			case self::TYPE_VERSION:
				list(,$version) = unpack('q', fread($this->fp, 8));
				$pkt['version'] = $version;
				return $pkt;
		}
	}

	public function getPacketBody($id) {
		$pkt = $this->getPacket($id);
		fseek($this->fp, $pkt['body_offset']);
		return fread($this->fp, $pkt['body']);
	}

	public function buildIndex() {
		fseek($this->fp, 0, SEEK_END);
		$size = ftell($this->fp);

		fseek($this->fp, 0);
		$index = [
			'url' => [], // url to stream(s)
			'stream' => [], // stream to packets
			'pkt' => [], // packet to file offset
		];

		$pkt_idx = 0;
		$version = 0;

		while(true) {
			$pkt_idx += 1;
			$ls = fread($this->fp, 8);
			if (strlen($ls) == 0) {
				$pkt_idx -= 1;
				break; // eof?
			}
			if (strlen($ls) != 8) throw new \Exception('Could not parse file (partial frame?)');
			list(, $l) = unpack('P', $ls);

			// check against file size
			if (ftell($this->fp) + $l > $size) {
				//throw new \Exception('Could not parse file (data error or partial frame)');
				$pkt_idx -= 1;
				break; // reached EOF with a partial frame :(
			}

			$offset = ftell($this->fp);

			// read header
			$info = unpack('Ctype/Ptime/Pid', fread($this->fp, 1+8+8));
			$info['offset'] = $offset;
			$info['length'] = $l;
			$info['body'] = $l-17; // size minus header
			$info['body_offset'] = ftell($this->fp);

			$index['pkt'][$pkt_idx] = $info;

			if ($info['id'] == 0) {
				if ($info['type'] == self::TYPE_VERSION) {
					// store new version
					list(,$version) = unpack('q', fread($this->fp, 8));
				}
				fseek($this->fp, $offset + $l);
				continue; // skip further registration
			}

			$id = $info['id'];
			if (!isset($index['stream'][$id]))
				$index['stream'][$id] = [];
			$index['stream'][$id][] = $pkt_idx;

			if ($info['type'] == self::TYPE_REQ) {
				// store url pointer to stream
				$method = stream_get_line($this->fp, 4096, "\0");
				$url = stream_get_line($this->fp, 4096, "\0");
				
				$index['url'][$url][$method][] = $id;
			}

			fseek($this->fp, $offset + $l);
		}

		$index['last_packet'] = $pkt_idx;

		$this->index = $index;
	}
}
