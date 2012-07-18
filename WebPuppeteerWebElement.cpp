#include "WebPuppeteerWebElement.hpp"

WebPuppeteerWebElement::WebPuppeteerWebElement(QWebElement el) {
	e = el;
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
