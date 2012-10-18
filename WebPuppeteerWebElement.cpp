#include "WebPuppeteerWebElement.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QScriptValueIterator>
#include <QWebFrame>

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

bool WebPuppeteerWebElement::hasAttribute(const QString &name) {
	return e.hasAttribute(name);
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

QScriptValue WebPuppeteerWebElement::parentNode() {
	QWebElement el = e.parent();
	if (el.isNull()) return parent->getParent()->engine().nullValue();
	return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership);
}

QScriptValue WebPuppeteerWebElement::firstChild() {
	QWebElement el = e.firstChild();
	if (el.isNull()) return parent->getParent()->engine().nullValue();
	return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership);
}

QScriptValue WebPuppeteerWebElement::nextSibling() {
	QWebElement el = e.nextSibling();
	if (el.isNull()) return parent->getParent()->engine().nullValue();
	return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership);
}

bool WebPuppeteerWebElement::click() {
	return e.evaluateJavaScript("(function(obj) { var e = document.createEvent('MouseEvents'); e.initEvent('click',true,false); return obj.dispatchEvent(e); })(this)").toBool();
}

bool WebPuppeteerWebElement::onblur() {
	return e.evaluateJavaScript("(function(obj) { var e = document.createEvent('MouseEvents'); e.initEvent('blur',true,false); return obj.dispatchEvent(e); })(this)").toBool();
}

bool WebPuppeteerWebElement::onchange() {
	return e.evaluateJavaScript("(function(obj) { var e = document.createEvent('MouseEvents'); e.initEvent('change',true,false); return obj.dispatchEvent(e); })(this)").toBool();
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
			str = el.attribute("Value")+el.toPlainText();
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

QScriptValue WebPuppeteerWebElement::find(QScriptValue v) {
	QList<QWebElement> c = allChildren();
	QScriptValue res = parent->getParent()->engine().newArray();
	int res_pos = 0;

	QMap<QString,QString> s;
	if (!v.isObject()) return parent->getParent()->engine().nullValue();
	QScriptValueIterator vi(v);
	while(vi.hasNext()) {
		vi.next();
		s.insert(vi.name().toLower(), vi.value().toString());
	}

	for(int i = 0; i < c.size(); i++) {
		QWebElement el = c.at(i);
		bool ok = true;

		for(QMap<QString,QString>::iterator j = s.begin(); j != s.end(); j++) {
			if (j.key() == "tagname") {
				if (j.value().toUpper() != el.tagName()) {
					ok = false;
					break;
				}
				continue;
			}
			if (el.attribute(j.key()) != j.value()) {
				ok = false;
				break;
			}
		}
		if (!ok) continue;

		res.setProperty(res_pos++, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership));
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

QScriptValue WebPuppeteerWebElement::getElementsByTagName(const QString &tag) {
	QList<QWebElement> c = allChildren();

	QScriptValue res = parent->getParent()->engine().newArray();
	int res_pos = 0;

	QString real_tag = tag.toUpper();

	for(int i = 0; i < c.size(); i++) {
		QWebElement el = c.at(i);

		if (el.tagName() != real_tag) continue;
		res.setProperty(res_pos++, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership));
	}

	return res;
}
QScriptValue WebPuppeteerWebElement::getElementsByName(const QString &name) {
	QList<QWebElement> c = allChildren();

	QScriptValue res = parent->getParent()->engine().newArray();
	int res_pos = 0;

	for(int i = 0; i < c.size(); i++) {
		QWebElement el = c.at(i);

		if (el.attribute("name") != name) continue;
		res.setProperty(res_pos++, parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, el), QScriptEngine::ScriptOwnership));
	}

	return res;
}

QString WebPuppeteerWebElement::textContent() {
	return e.toPlainText();
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

QString WebPuppeteerWebElement::tagName() {
	return e.tagName();
}

void WebPuppeteerWebElement::setFocus() {
	e.setFocus();
}

bool WebPuppeteerWebElement::hasFocus() {
	return e.hasFocus();
}

QScriptValue WebPuppeteerWebElement::frameDocument(QString framename) {
	if (e.webFrame() == NULL) return QScriptValue(QScriptValue::NullValue);

	QWebFrame *frame = e.webFrame();
	QList<QWebFrame *> fl = frame->childFrames();

	for(int i = 0; i < fl.size(); i++) {
		if (fl.at(i)->frameName() == framename) return parent->getParent()->engine().newQObject(new WebPuppeteerWebElement(parent, fl.at(i)->documentElement()), QScriptEngine::ScriptOwnership);
	}

	return QScriptValue(QScriptValue::NullValue);
}

