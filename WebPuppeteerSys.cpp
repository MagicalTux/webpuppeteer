#include "WebPuppeteerSys.hpp"
#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>
#include <QDateTime>
#include <QFile>
#include "sha2.h"

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

	rep->deleteLater();
	return parent->engine().newVariant((rep->readAll()));
}

QScriptValue WebPuppeteerSys::signedPost(const QString &url, const QString &post, const QString &api_key, const QString &api_secret) {
	// compatible with MtGox API
	static quint64 last_once = 0;
	quint64 once = QDateTime::currentMSecsSinceEpoch();
	if (once <= last_once) {
		once = last_once+1;
	}
	last_once = once;

	QByteArray data_post;
	if (post == "") {
		data_post = (post + QString("nonce=%1").arg(once)).toUtf8();
	} else {
		data_post = (post + QString("&nonce=%1").arg(once)).toUtf8();
	}


	QNetworkRequest req(url);
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	req.setRawHeader("Rest-Key", api_key.toLatin1());

	// quick implementation of SHA512 hmac
	SHA512_CTX ctx;
	quint8 sha512[SHA512_DIGEST_LENGTH];
	QByteArray bin_api_secret(QByteArray::fromBase64(api_secret.toLatin1()));

	if (bin_api_secret.size() > SHA512_BLOCK_LENGTH) {
		SHA512_Init(&ctx);
		SHA512_Update(&ctx, (quint8*)bin_api_secret.data(), bin_api_secret.size());
		SHA512_Final(sha512, &ctx);
		bin_api_secret = QByteArray((char*)&sha512, SHA512_DIGEST_LENGTH);
	}
	if (bin_api_secret.size() < SHA512_BLOCK_LENGTH) {
		bin_api_secret += QByteArray(bin_api_secret.size() - SHA512_BLOCK_LENGTH, 0);
	}

	QByteArray o_key_pad = QByteArray(SHA512_BLOCK_LENGTH, 0x5c);
	QByteArray i_key_pad = QByteArray(SHA512_BLOCK_LENGTH, 0x36);

	for(int i = 0; i < SHA512_BLOCK_LENGTH; i++) {
		o_key_pad[i] = o_key_pad[i] ^ bin_api_secret[i];
		i_key_pad[i] = i_key_pad[i] ^ bin_api_secret[i];
	}

	// first round of HMAC
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, (quint8*)i_key_pad.data(), i_key_pad.size());
	SHA512_Update(&ctx, (quint8*)data_post.data(), data_post.size());
	SHA512_Final(sha512, &ctx);
	bin_api_secret = QByteArray((char*)&sha512, SHA512_DIGEST_LENGTH);
	// second round of HMAC
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, (quint8*)o_key_pad.data(), o_key_pad.size());
	SHA512_Update(&ctx, (quint8*)bin_api_secret.data(), bin_api_secret.size());
	SHA512_Final(sha512, &ctx);
	bin_api_secret = QByteArray((char*)&sha512, SHA512_DIGEST_LENGTH);

	req.setRawHeader("Rest-Sign", bin_api_secret.toBase64());

	QNetworkReply *rep = net.post(req, data_post);
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

void WebPuppeteerSys::quit() {
	parent->exit(0);
}

void WebPuppeteerSys::abort() {
	parent->exit(1);
}

bool WebPuppeteerSys::filePutContents(QString filename, QString data) {
	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) return false;
	f.write(data.toUtf8());
	f.close();
	return true;
}

