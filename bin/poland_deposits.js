// poland
sys.include("lib/centrum24.js");
sys.include("config.js");

var b = new centrum24();
b.login(centrum_nik, centrum_password);
//b.interact();
var later_do = [];

var cb = function (trx) {
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/centrum24_push", "json="+encodeURIComponent(JSON.stringify(trx)), api_key, api_secret);
	res = JSON.parse(res);
	if (res.result != "success") {
		sys.log(JSON.stringify(res));
		sys.abort();
	}
	sys.log("TX ["+trx.desc+"]: "+res["return"]["status"]);
	if (res["return"]["status"] == "original_tx_missing") {
		later_do.unshift(trx);
		return true;
	}
	if (res["return"]["status"] == "already_in_db") return false;

	if (trx.desc.substr(0,14) == "MtGox Withdraw") return true; // no need for check

	// try to run later_do list too (up to 10 entries)
	for(var i = 0; i < Math.min(later_do.length, 10); i++) {
		var trx = later_do[i];
		var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/centrum24_push", "json="+encodeURIComponent(JSON.stringify(trx)), api_key, api_secret);
		res = JSON.parse(res);
		if (res.result != "success") {
			sys.log(JSON.stringify(res));
			sys.abort();
		}
		sys.log(" * TX ["+trx.desc+"]: "+res["return"]["status"]);
		if (res["return"]["status"] == "already_in_db") {
			later_do.splice(i,1);
			continue;
		}
		if (res["return"]["status"] == "original_tx_missing") continue;
		// something happened, remove it!
		later_do.splice(i,1);
		break;
	}

	return true;
//	return (res["return"]["status"] != "already_in_db");
};

try {
	b.scanAccountByNumber("87 1090 2398 0000 0001 1759 5675", cb); // PLN
	b.scanAccountByNumber("59 1090 2398 0000 0001 1759 5694", cb); // EUR
//	b.scanAccountByNumber("58 1090 2398 0000 0001 1759 5712", cb); // USD
} catch(e) {
	sys.log("Error: "+e);
}

try {
	b.logout();
} catch(e) {
}

for(var i = 0; i < later_do.length; i++) {
	var trx = later_do[i];
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/centrum24_push", "json="+encodeURIComponent(JSON.stringify(trx)), api_key, api_secret);
	res = JSON.parse(res);
	if (res.result != "success") {
		sys.log(JSON.stringify(res));
		sys.abort();
	}
	sys.log(" . TX ["+trx.desc+"]: "+res["return"]["status"]);
}

