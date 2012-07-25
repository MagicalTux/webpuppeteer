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
	if (res["return"]["status"] == "original_tx_missing")
		later_do.push(trx);
	return (res["return"]["status"] != "already_in_db");
};

b.scanAccountByNumber("87 1090 2398 0000 0001 1759 5675", cb); // PLN
b.scanAccountByNumber("59 1090 2398 0000 0001 1759 5694", cb); // EUR

b.logout();

for(var i = 0; i < later_do.length; i++) {
	var trx = later_do[i];
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/centrum24_push", "json="+encodeURIComponent(JSON.stringify(trx)), api_key, api_secret);
	res = JSON.parse(res);
	if (res.result != "success") {
		sys.log(JSON.stringify(res));
		sys.abort();
	}
	sys.log("TX ["+trx.desc+"]: "+res["return"]["status"]);
}

