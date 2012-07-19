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
	return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership);
}

QScriptValue WebPuppeteerWebElement::findAll(const QString &selector) {
	QScriptValue res = parent->getParent()->engine().newArray();
	QWebElementCollection c = e.findAll(selector);
	for(int i = 0; i < c.count(); i++) {
		res.setProperty(i, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, c.at(i)), QScriptEngine::ScriptOwnership));
	}
	return res;
}

bool WebPuppeteerWebElement::click() {
	return e.evaluateJavaScript("(function(obj) { var e = document.createEvent('MouseEvents'); e.initEvent('click',true,false); return obj.dispatchEvent(e); })(this)").toBool();
}

QScriptValue WebPuppeteerWebElement::findAllContaining(const QString &text) {
	QList<QWebElement> c = allChildren();

	QScriptValue res = parent->getParent()->engine().newArray();
	int res_pos = 0;

	for(int i = 0; i < c.size(); i++) {
		QWebElement el = c.at(i);

		bool check = false;
		QString str;

		if ((el.tagName() == "A") && (el.hasAttribute("href"))) {
			str = el.toPlainText();
			check = true;
		} else if (el.tagName() == "BUTTON") {
			str = el.toPlainText();
			check = true;
		} else if ((el.tagName() == "INPUT") && (el.attribute("type").toLower() == "submit")) {
			str = el.attribute("value");
			check = true;
		}

		if ((check) && (str.contains(text, Qt::CaseInsensitive))) {
			res.setProperty(res_pos++, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership));
		}
	}

	return res;
}

QScriptValue WebPuppeteerWebElement::getElementById(const QString &id) {
	QList<QWebElement> c = allChildren();

	for(int i = 0; i < c.size(); i++) {
		QWebElement el = c.at(i);
		if (!el.hasAttribute("id")) continue;

		if (el.attribute("id") == id) {
			return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership);
		}
	}
	return parent->getParent()->engine().nullValue();
}

QList<QWebElement> WebPuppeteerWebElement::allChildren() {
	// return all childrens, even grandchildrens
	QWebElement el = e.firstChild();
	QList<QWebElement> res;

	while(true) {
		res.append(el);

		QWebElement tmp = el.firstChild();
		if (!tmp.isNull()) {
			el = tmp;
			continue;
		}
		tmp = el.nextSibling();
		if (!tmp.isNull()) {
			el = tmp;
			continue;
		}

		bool exit = false;
		while(true) {
			tmp = el.parent();
			if ((tmp.isNull()) || (tmp == e)) {
				exit = true;
				break;
			}
			el = tmp;
			tmp = el.nextSibling();
			if (!tmp.isNull()) {
				el = tmp;
				break;
			}
		}
		if (exit) break;
	}

	return res;
}

