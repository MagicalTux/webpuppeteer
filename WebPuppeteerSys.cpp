#include "WebPuppeteerSys.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>

WebPuppeteerSys::WebPuppeteerSys(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
}

void WebPuppeteerSys::log(const QString &msg) {
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

QScriptValue WebPuppeteerSys::get(const QString &url) {
	QNetworkRequest req(url);
	QNetworkReply *rep = net.get(req);
	QEventLoop e;

	connect(rep, SIGNAL(finished()), &e, SLOT(quit()));
	e.exec();

	if (rep->error() != QNetworkReply::NoError) {
		qDebug("GET error: %s", qPrintable(rep->errorString()));
		rep->deleteLater();
		return parent->engine().currentContext()->throwError(QScriptContext::UnknownError, rep->errorString());
	}

	return parent->engine().newVariant((rep->readAll()));
}

