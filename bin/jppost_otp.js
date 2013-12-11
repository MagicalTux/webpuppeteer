// Japan Post
sys.include("config.js");

var tab = sys.newTab();
tab.trustCertificate("2578415491d65777018ac26cdc046831bfe80ee4"); // https://www.int-mypage.post.japanpost.jp
tab.load("https://www.int-mypage.post.japanpost.jp/mypage/M010000.do");

tab.document().getElementById('M010000_loginBean_id').setAttribute("value", jppost_email);
tab.document().getElementById('M010000_loginBean_pw').setAttribute("value", jppost_password);

tab.document().findFirst("form#M010000 table.layout tr td a img").click();
tab.wait();

res = JSON.parse(sys.signedPost("https://mtgox.com/api/1/generic/otp/prepare", "", api_key, api_secret));

var count = 0;

while(true) {
	count = count + 1;
	if (res.result != "success") {
		sys.log(JSON.stringify(res));
		sys.abort();
	}
	if (res["return"]["res_pdf"]) {
		// push it to printer
		sys.filePutContentsB64("/home/magicaltux/Downloads/pdf_otp_order_"+res["return"]["res_item"]+".pdf", res["return"]["res_pdf"]);
		if (count >= 49) break; // about 50 cards
	}

	if (!res["return"]["item"]) break;

	sys.log("Handling order "+res["return"]["item"]+" for "+res["return"]["item_name"]);

	var post = res["return"]["post"];
	var is_gift = res["return"]["gift"];

	if (post["country"] != "JP") {
		try {
			tab.document().findAllContaining("Make Label")[0].click();
			tab.wait();
			tab.document().findFirst("div.mrgT10 > input.button").click();
			tab.wait();
			tab.document().getElementById("M060400_sel-1").click();
			tab.wait();
			// input receipient addr from post
			tab.document().getElementById("M060505_addrToBean_nam").setAttribute("value", post["name"]);
			tab.document().getElementById("M060505_addrToBean_companyName").setAttribute("value", post["company_name"]);
			if (post["address2"]) {
				tab.document().getElementById("M060505_addrToBean_add1").setAttribute("value", post["address"].substr(0,80));
				tab.document().getElementById("M060505_addrToBean_add2").setAttribute("value", post["address2"].substr(0,80));
			} else {
				tab.document().getElementById("M060505_addrToBean_add2").setAttribute("value", post["address"].substr(0,80));
			}
			tab.document().getElementById("M060505_addrToBean_add3").setAttribute("value", post["city"]);
			tab.document().getElementById("M060505_addrToBean_pref").setAttribute("value", post["province"].substr(0,24));
			tab.document().getElementById("M060505_addrToBean_postal").setAttribute("value", post["zip"]);
			tab.document().getElementById("M060505_addrToBean_couCode").eval("this.value="+JSON.stringify(post["country"]));
			tab.document().getElementById("M060505_addrToBean_tel").setAttribute("value", post["phone"].replace(/[()]/g,''));
			tab.document().getElementById("M060505_addrToBean_fax").setAttribute("value", post["fax"]);
			tab.document().findFirst("input.button").click();
			tab.wait();
	
			tab.document().getElementById("M060800_shippingBean_sendType1").click();
			tab.document().getElementsByName("item_button01")[0].click();
			tab.wait();
			tab.document().getElementById("M080100_itemBean_pkg").setAttribute("value", "MTGOX OTP CARD");
			tab.document().getElementById("M080100_itemBean_couCd").eval("this.value='CH'");
			tab.document().getElementById("M080100_itemBean_cost_value").setAttribute("value", "39.99");
			tab.document().getElementById("curUnit").eval("this.value='USD'");
			tab.document().findFirst("input.button").click();
			tab.wait();
			tab.document().findFirst("input.button").click();
			tab.wait();
			tab.document().findFirst("input.button").click();
			tab.wait();
			tab.document().getElementById("M060800_itemCount").setAttribute("value", "1");
			if (is_gift) {
				tab.document().getElementById("M060800_shippingBean_pkgType").eval("this.value='0'"); // 3=merchandise 0=gift
			} else {
				tab.document().getElementById("M060800_shippingBean_pkgType").eval("this.value='3'"); // 3=merchandise 0=gift
			}
			tab.document().getElementById("M060800_shippingBean_pkgTotalPrice_value").eval("this.value='1200'");
			tab.document().getElementById("M060800_ShippingBean_danger").click();
			tab.document().findFirst("td > input.button").click();
			tab.wait();
	
			tab.document().getElementById("M060900_shippingBean_totalWeight_value").eval("this.value='36'");
			tab.document().findFirst("input.button").click();
			tab.wait();
			tab.document().findFirst("div.mrgT10 > table > tbody > tr > td > input.button").click();
			tab.wait();
			tab.document().findFirst("input.button").click();
			tab.wait();
//			tab.document().findFirst("input.button").click();
//			tab.wait();
	
			// get all downloaded files
			sys.log("waiting for files download");
			var cnt = 1;
			while(cnt) {
				var file = tab.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				sys.log("Got file: "+file.filename);
				sys.filePutContentsB64("/home/magicaltux/Downloads/"+file.filename, file.filedata);
				cnt--;
			}
	
			// get tracking number
			var ems_tracking = tab.document().findFirst("div.mrgT10 tr td.le div").textContent().replace(/\s/,"");
	
			// click da button!
			tab.document().findFirst("input.button").click();
			tab.wait();
		} catch(e) {
			sys.log(e);
			tab.interact();
			sys.abort();
		}
	} else {
		sys.log("JAPANESE CUSTOMER, no EMS, no tracking");
		var ems_tracking = "N/A";
	}

	var post_o = {
		order_item: res["return"]["item"],
		tracking: ems_tracking,
	};

	var post = "";
	for(i in post_o) {
		post += (post == ""?"":"&") + i + "=" + encodeURIComponent(post_o[i]);
	}

	res = JSON.parse(sys.signedPost("https://mtgox.com/api/1/generic/otp/prepare", post, api_key, api_secret));
	sys.log("");
}

