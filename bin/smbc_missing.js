sys.include("config.js");
// from https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_SKBCNTL?action=action3&refreshRandomNum=862&UkeID=ACHE9H001

function web21_encode_password(table, password) {
	var retStr = "";
	var t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	var randomNum = table[table.length-1];
	var table1Pos = parseInt(randomNum.substring(0,1));
	var table2Pos = parseInt(randomNum.substring(1,2));
	var table3Pos = parseInt(randomNum.substring(2,3));

	for(i = 0; i < password.length; i++) {
		var temp = password.substring(i,i+1);
		index = t.indexOf(temp);
		temp= table[62 * table1Pos + index];
		index = t.indexOf(temp);
		temp= table[62 * table2Pos + index];
		index = t.indexOf(temp);
		temp= table[62 * table3Pos + index];
		retStr = retStr + temp;
	}
	return retStr;
}

function web21_picktable(table) {
	var picktmp = "";
	for(i = 0; i < 10; i++){
		picktmp= picktmp + table[62 * i + 0];
		picktmp= picktmp + table[62 * i + 30];
		picktmp= picktmp + table[62 * i + 61];
	}
	return picktmp;
}

var tab = sys.newTab();
tab.overrideUserAgent("Mozilla/5.0 (Windows; U; MSIE 9.0; Windows NT 9.0; en-US)"); // make it look like MSIE9
tab.setDefaultCharset("SJIS");
tab.load("https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_SKBCNTL?action=action3&refreshRandomNum=862&UkeID=ACHE9H001");
var table = tab.document().textContent();
table = table.substring(table.indexOf("RET_VALUE=") + 10).split(",");

tab.post("https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_WEBCNTL", "User="+valuedoor_id+"&Pwd1="+web21_encode_password(table, valuedoor_password)+"&hid1=&UkeID=ACHE9H001&APNextScrID=ACH999002&pickTable="+web21_picktable(table)+"&_W_SoftKeyBoardFlag=1");

tab.document().findAllContaining("Global e-Trade サービス")[0].click();
tab.wait();
tab.waitFinish();

while(true) {
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/smbc_missing_data", "", api_key, api_secret);
	res = JSON.parse(res);
	res = res["return"];
	sys.log("handling id "+res.id+" dated "+res.date);
	var data = {};
	var meta = {};
	var skip = false;
	switch(res.type) {
		case "withdraw":
			sys.log("withdraw index = W"+res.index);
			var files = {};
			// get fee related stuff
			if (!tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mKeisanshoJikkohYmdFrmYY")[0]) {
				var t = tab.document().frameDocument("menu").frameDocument("menu2").find({tagname:"img", alt: "計算書照会"});
				t[1].click();
				tab.waitFinish();
			}
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mKeisanshoJikkohYmdFrmYY")[0].setAttribute("value", "2011");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mKokyakuSeiriNo")[0].setAttribute("value", "W"+res.index);
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mListSearchBt")[0].click();
			tab.waitFinish();
			// check the box
			var checkbox = tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mTTCalcTableBlock_mCheckBox")[0];
			if (!checkbox) {
				sys.log("no detail, skipping");
				skip = true;
				break;
			}
			checkbox.eval("this.checked = true");
			// get the SMBC transfer id (Receipt)
			var td = checkbox.parentNode();
			while(td.nextSibling()) td = td.nextSibling();
			var id = td.textContent();
			sys.log("got id = "+id);
			data["receipt"] = id;

			// download request form
			sys.log("Downloading transfer request form...");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mTTCalcTableBlock_mIraiShowBt")[0].click();

			var file;

			while(true) {
				file = tab.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				break;
			}
			files["request_form.pdf"] = file.filedata;

			sys.log("Downloading fee statement...");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTCalcListBlock_mTTCalcTableBlock_mStatementShowBt")[0].click();

			while(true) {
				file = tab.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				break;
			}

			files["fee_statement.pdf"] = file.filedata;

			var t = tab.document().frameDocument("menu").frameDocument("menu2").find({tagname:"img", alt: "全件明細照会"});
			t[1].click();
			tab.waitFinish();

			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mSohkinTorikumiYmdFrmYY")[0].setAttribute("value", "2011");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mKokyakuSeiriNo")[0].setAttribute("value", "W"+res.index);
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mShoriKbnBox")[0].eval("this.value = 6");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mListSearchBt")[0].click();
			tab.waitFinish();
			// check the box
			if (tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mTTAllDetailTableBlock_mSelectNo").length) {
				tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mTTAllDetailTableBlock_mSelectNo")[0].eval("this.checked = true");

				// download SWIFT tx
				sys.log("Downloading SWIFT transaction...");
				tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mTTAllDetailListBlock_mTTAllDetailTableBlock_mSoukinResultShow")[0].click();

				var s_time = ((new Date()).getTime()) + 10000; // 10 secs

				while(true) {
					file = tab.getDownloadedFile();
					if (!file) {
						sys.sleep(100);
						if (s_time < ((new Date()).getTime())) {
							sys.log("timeout on SWIFT download, probably no swift document available");
							break;
						}
						continue;
					}
					break;
				}
				if (file)
					files["swift.pdf"] = file.filedata;
			}

			data["files"] = files;

			break;
		case "deposit":
			sys.log("deposit key = "+res.key);
			if (res.key.substr(0, 1) == "-") {
				sys.log("domestic, skipping");
				skip = true;
				break;
			}
			var files = {};
			// get fee related stuff
//			if (!tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mTohshoNyuukinYmdFrmYY")[0]) {
				var t = tab.document().frameDocument("menu").frameDocument("menu2").find({tagname:"img", alt: "計算書照会"});
				t[0].click();
				tab.waitFinish();
//			}
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mTohshoNyuukinYmdFrmYY")[0].setAttribute("value", "2011");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mTohkohSeiriNo")[0].setAttribute("value", res.key);
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mListSearchBt")[0].click();
			tab.waitFinish();
			var checkbox = tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mRTCalcTableBlock_mCheckBox")[0];
			if (!checkbox) {
				sys.log("no detail, skipping");
				skip = true;
				break;
			}
			checkbox.eval("this.checked = true");

			// download fee statement
			sys.log("Downloading fee statement...");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mRTCalcTableBlock_mStatementShowBt")[0].click();

			var file;

			while(true) {
				file = tab.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				break;
			}
			files["fee_statement.pdf"] = file.filedata;

			var t = tab.document().frameDocument("menu").frameDocument("menu2").find({tagname:"img", alt: "全件明細照会"});
			t[0].click();
			tab.waitFinish();

			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mAnnaiYmdFrmYY")[0].setAttribute("value", "2011");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mTohkohSeiriNo")[0].setAttribute("value", res.key);
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mListSearchBt")[0].click();
			tab.waitFinish();

			// download transfer form
			sys.log("Downloading TT form...");
			tab.document().frameDocument("service").frameDocument("main").getElementsByName("root_mRTCalcListBlock_mRTCalcTableBlock_mPDFShowBt")[0].click();

			while(true) {
				file = tab.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				break;
			}
			files["tt_details.pdf"] = file.filedata;

			data["files"] = files;

			break;
		default:
			sys.log("Unknown data: "+JSON.stringify(res));
			sys.quit();
	}

	var final_data = res;
	final_data["skip"] = skip;
	final_data["data"] = data;
	final_data["meta"] = meta;

	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/smbc_missing_data_push", "data="+encodeURIComponent(JSON.stringify(final_data)), api_key, api_secret);
	//tab.interact();
}

