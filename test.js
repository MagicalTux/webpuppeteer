// JS script for WebPuppeteer
// it can do stuff!

console.log("Loading google.com...");
tab = sys.newTab();
tab.browse("http://www.google.com/ncr");
console.log("loaded, messing with the page");

tab.type("cats");
tab.typeEnter();
tab.wait();

tab.document().findAllContaining("Images")[0].click();

tab.interact();

