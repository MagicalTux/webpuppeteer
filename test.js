// JS script for WebPuppeteer
// it can do stuff!

console.log("Loading google.com...");
tab = sys.newTab();

// dump all network traffic to this file
tab.saveNetwork("google_img.bin");

tab.browse("http://www.google.com/ncr");
console.log("loaded, messing with the page");

tab.type("cats");
tab.typeEnter();
tab.wait();

tab.document().findAllContaining("Images")[0].click();
tab.wait();

tab.interact();

