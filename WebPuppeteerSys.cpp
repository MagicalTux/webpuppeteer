#include "WebPuppeteerSys.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>

WebPuppeteerSys::WebPuppeteerSys(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
}

void WebPuppeteerSys::log(QString msg) {
	qDebug("%s", qPrintable(msg));
}

void WebPuppeteerSys::sleep(int msecs) {
	QEventLoop e;
	QTimer::singleShot(msecs, &e, SLOT(quit()));
	e.exec();
}

QScriptValue WebPuppeteerSys::newTab() {
	// open a new "browser tab", return it
	WebPuppeteerTab *tab = new WebPuppeteerTab(parent);

	return parent->engine().newQObject(tab, QScriptEngine::ScriptOwnership, QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeSuperClassContents);
}

