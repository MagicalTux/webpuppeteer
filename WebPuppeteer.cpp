#include <QCoreApplication>
#include <QFile>
#include <QScriptEngine>
#include "WebPuppeteer.hpp"
#include "WebPuppeteerSys.hpp"

WebPuppeteer::WebPuppeteer(QString _file) {
	file = _file;
	exit_code = 0;
	sys = new WebPuppeteerSys(this);

	QScriptValue val_sys = e.newQObject(sys, QScriptEngine::QtOwnership, QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater);
	e.globalObject().setProperty("sys", val_sys);
	e.globalObject().setProperty("console", val_sys);
}

void WebPuppeteer::start() {
	QFile f(file);
	if (!f.exists()) {
		qDebug("script does not exist");
		QCoreApplication::exit(1);
		return;
	}

	if (!f.open(QIODevice::ReadOnly)) {
		qDebug("script could not be opened");
		QCoreApplication::exit(2);
		return;
	}

	QScriptValue res = e.evaluate(QString::fromUtf8(f.readAll()), file);

	if (res.isError()) {
		qDebug("Error while executing script: %s", qPrintable(res.toString()));
		exit_code = 1;
	}

	QCoreApplication::exit(exit_code);
}

