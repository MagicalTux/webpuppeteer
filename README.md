# WebPuppeteer

This project is about allowing a simple ECMA-compliant script to call pages in a browser environment, and interact with said pages as it pleases.

This project helps with two different goals:

* Allow performing tests on existing websites by accessing said sites, clicking, etc, as a normal user would and report any resulting issue
* Allow automation of access to third parties (banks, etc) when no automation API is available

I've been using Selenium IDE with Firefox for a long time, but in some cases is lacked the easy access to custom operations that webpuppeteer provides with ECMA scripting. While more limited in some ways (Webkit only) this projects offers more freedom in what can be done, including altering the page contents itself or running JavaScript within the context of the browser.

WebPuppeteer can also generate screenshots or PDF of webpages to document results or issues. It can also record all network activity (see saveNetwork()) making it easy to create audit records that can be confirmed afterward. Be careful all network activity is saved and those files may contain sensitive information depending on how you use them.

## Installation

Pre-requisites can be installed easily, for example for Ubuntu:

	sudo apt install qt5-qmake qtscript5-dev libqt5webkit5-dev

The compilation is easy:

	qmake -qt=5
	make
	make install

## Headless?

Running headless is easy, just use xvfb.

To have xvfb on Gentoo:

- x11-base/xorg-server minimal

# Tell me what that scripts can do

There are generally three objects scripts will interact with. WebPuppeteerSys is the root object (available in global variable sys) and allows basic control as well as instanciating tabs. A "tab" is similar to a browser tab and can be used to browse to urls, take screenshots, etc. Finally, accessing a tab's root element allows searching for sub-elements, and even clicking these.

## WebPuppeteerSys

Initially you have access to this object in "sys" global (also aliased as "console" so you can type console.log(...) without any issue). This object has the following methods.

### log(message)

Display a message on the console.

```javascript
sys.log("Ready!");
```

### sleep(msec)

Wait for a specified number of milliseconds (for example to avoid making too many queries against a given website).

For example to wait one second:
```javascript
sys.sleep(1000);
```

### newTab()

Opens a new tab and returns a new WebPuppeteerTab object (see below).

### get(url)

Get data from url, returns it.

```javascript
var next = JSON.parse(sys.get("http://localhost/next.php"));
```

## WebPuppeteerTab

A WebPuppeteerTab is a browser tab instance. Each tab is independent from each other, and can be accessing different websites at the same time.

### browse(url)

Browse to given url (paths relative to the current page are acceptable), waits until url is fully loaded.

```javascript
var tab = sys.newTab();
tab.browse("https://www.google.com/");
```

### post(url, post data, content type)

Posts data to url as if submitted by browser. content type is application/x-www-form-urlencoded by default.

### wait()

Wait for page load operations to finish (use for example after clicking a link or a button).

### waitFinish()

Wait for page load operations to finish, with a bit more time in case something loads after page load ends (redirect, etc).

### getNewWindow()

In case a script/etc opened a new window, gets that window (returns a WebPuppeteerTab object).

### back()

Go back one page (equivalent of browser back button).

### reload(force no cache)

Reload page. If force no cache is true, local cache will be ignored.

### screenshot(filename)

Take a screenshot of the current page at current resolution.

### fullshot(filename)

Take a full-size screenshot of the page, even if it extends over the current resolution.

### print(filename)

Outputs current page as PDF.

### printBase64()

Same as print, but returns base64 encoded PDF instead of writing to file.

### saveNetwork(filename)

Starts saving all network activity to filename, including loaded URLs and returned data, allowing for later reproduction of the same page load.

### eval(javascript)

Execute javascript code in the page.

### get(url)

Get url in the context of the web page, and return contents.

### overrideUserAgent(ua)

Sets user agent to ua

### setDefaultCharset(charset)

Change default charset in page loading.

### getDownloadedFile()

Returns latest downloaded file as an array with "filename" (name of file) and "filedata" (base64 encoded downloaded data). Not suitable for large files.

### document()

Returns the document element.

### treeDump()

Dump webkit's internal rendering tree.

### getHtml()

Return page html as string.

### type(text)

Cause text to be inputted as if typed by someone on a keyboard.

### typeEnter()

Cause keypress of enter key.

### typeTab()

Cause keypress of tab key.

### interact()

Opens the page in a window, and give the user opportunity to interact with the page.

## WebPuppeteerWebElement

This class represents one element inside the page. It acts in a similar way to DOM.

### attribute(name)

Get attribute value.

### setAttribute(name, value)

Set a given attribute.

### hasAttribute(name)

Check if has an attribute.

### xml()

Return element code as xml.

### textContent()

Return element text content.

### eval(js)

Eval javascript in context of element.

### click()

Cause click event to happen on element.

### onblur()

Cause blur event to happen on element.

### onchange()

Cause change event to happen on element.

### setStyleProperty(name, value)

Set css style property.

### getComputedStyle(name)

Get css property name as computed.

### tagName()

Return element's tag name.

### find(conditions)

Find all elements matching conditions set. For example: find({tagname:"a", id:"test"})

### findFirst(selectorQuery)

Locate first element matching the CSS2 selector passed as argument.

### findAll(selectorQuery)

Locate all elements matching the CSS2 selector passed as argument.

### findAllContaining(text)

Locate all elements containing the given string.

### getElementById(id)

Same as DOM.

### getElementsByTagName(tag)

Same as DOM.

### getElementsByName(name)

Same as DOM.

### parentNode()

### firstChild()

### nextSibling()

### frameDocument(framename)

Get root document object for a given frame directly in the object's context.

### setFocus()

Set focus on given object (for example if input text, any input will happen there).

### hasFocus()

Returns true if given element has focus.
