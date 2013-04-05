// poland
sys.include("config.js");

var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/centrum24_pull_pln", "", api_key, api_secret);
sys.log(res);
res = JSON.parse(res);
sys.filePutContentsB64("poland_withdraw.csv", res["return"]["data"]);

