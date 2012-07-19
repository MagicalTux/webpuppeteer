#include "WebPuppeteerWebElement.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"

WebPuppeteerWebElement::WebPuppeteerWebElement(WebPuppeteerTab *_parent, QWebElement el) {
	e = el;
	parent = _parent;
}

QString WebPuppeteerWebElement::attribute(const QString &name) {
	return e.attribute(name);
}

void WebPuppeteerWebElement::setAttribute(const QString &name, const QString &value) {
	e.setAttribute(name, value);
}

QString WebPuppeteerWebElement::xml() {
	return e.toOuterXml();
}

void WebPuppeteerWebElement::eval(const QString &js) {
	e.evaluateJavaScript(js);
}

void WebPuppeteerWebElement::setStyleProperty(const QString &name, const QString &value) {
	e.setStyleProperty(name, value);
}

QString WebPuppeteerWebElement::getComputedStyle(const QString &name) {
	return e.styleProperty(name, QWebElement::ComputedStyle);
}

QScriptValue WebPuppeteerWebElement::findFirst(const QString &selector) {
	QWebElement el = e.findFirst(selector);
	if (el.isNull()) return parent->getParent()->engine().nullValue();
	return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el));
}

QScriptValue WebPuppeteerWebElement::findAll(const QString &selector) {
	QScriptValue res = parent->getParent()->engine().newArray();
	QWebElementCollection c = e.findAll(selector);
	for(int i = 0; i < c.count(); i++) {
		res.setProperty(i, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, c.at(i))));
	}
	return res;
}

