#include "WebPuppeteerSys.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>
#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QProcess>

WebPuppeteerSys::WebPuppeteerSys(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
	tmp_e = NULL;
}

void WebPuppeteerSys::log(const QString &msg) {
	qDebug("%s", qPrintable(msg));
}

void WebPuppeteerSys::sleep(int msecs) {
	QEventLoop e;
	QTimer::singleShot(msecs, &e, SLOT(quit()));
	e.exec();
}

QScriptValue WebPuppeteerSys::getenv(const QString &var) const {
	return QString::fromUtf8(qgetenv(qPrintable(var)));
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

	rep->deleteLater();
	return parent->engine().newVariant((rep->readAll()));
}

QScriptValue WebPuppeteerSys::post(const QString &url, const QString &post, const QString content_type) {
	QByteArray data_post;
	data_post = post.toUtf8();

	QNetworkRequest req(url);
	req.setHeader(QNetworkRequest::ContentTypeHeader, content_type);
	//req.setRawHeader("Rest-Key", api_key.toLatin1());

	QNetworkReply *rep = net.post(req, data_post);
	QEventLoop e;

	connect(rep, SIGNAL(finished()), &e, SLOT(quit()));
	e.exec();

	if (rep->error() != QNetworkReply::NoError) {
		qDebug("GET error: %s (body: %s)", qPrintable(rep->errorString()), qPrintable(rep->readAll()));
		rep->deleteLater();
		return parent->engine().currentContext()->throwError(QScriptContext::UnknownError, rep->errorString());
	}

	rep->deleteLater();
	return parent->engine().newVariant((rep->readAll()));
}

void WebPuppeteerSys::alert(QString string) {
	QMessageBox::information(NULL, "WebPuppeteer alert", string);
}

void WebPuppeteerSys::alertcb(QString string, QScriptValue cb) {
	tmp_cb = cb;
	QMessageBox mb(QMessageBox::Information, "WebPuppeteer alert", string, QMessageBox::Ok);
	QEventLoop e;
	QTimer t;
	tmp_e = &e;
	t.setSingleShot(false);
	t.setInterval(250);
	t.start();
	connect(&mb, SIGNAL(buttonClicked(QAbstractButton*)), &e, SLOT(quit()));
	connect(&t, SIGNAL(timeout()), this, SLOT(alertcb_cb()));
	mb.show();

	e.exec();
	tmp_e = NULL;
}

void WebPuppeteerSys::alertcb_cb() {
	if (tmp_e == NULL) return;
	if (tmp_cb.call().toBool()) tmp_e->quit();
}

bool WebPuppeteerSys::confirm(QString string) {
	return QMessageBox::question(NULL, "WebPuppeteer question", string, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
}

void WebPuppeteerSys::quit() {
	parent->exit(0);
}

void WebPuppeteerSys::abort() {
	parent->exit(1);
}

QString WebPuppeteerSys::fileGetContents(QString filename) {
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	return QString::fromUtf8(f.readAll());
}

QString WebPuppeteerSys::fileGetContentsB64(QString filename) {
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	return QString::fromLatin1(f.readAll().toBase64());
}

bool WebPuppeteerSys::filePutContents(QString filename, QString data) {
	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) return false;
	f.write(data.toUtf8());
	f.close();
	return true;
}

bool WebPuppeteerSys::filePutContentsB64(QString filename, QString data) {
	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) return false;
	f.write(QByteArray::fromBase64(data.toLatin1()));
	f.close();
	return true;
}

QScriptValue WebPuppeteerSys::include(QString filename) {
	QFile f(filename);
	if (!f.exists()) {
		return parent->engine().currentContext()->throwError(QScriptContext::RangeError, "Include file not found");
	}
	if (!f.open(QIODevice::ReadOnly)) {
		return parent->engine().currentContext()->throwError(QScriptContext::UnknownError, "Could not open file for reading");
	}
	return parent->engine().evaluate(QString::fromUtf8(f.readAll()), filename);
}

QString WebPuppeteerSys::exec(QString str) {
	QProcess p;
	p.start(str);
	p.waitForFinished();
	return QString::fromUtf8(p.readAll());
}

