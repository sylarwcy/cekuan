#pragma once
#ifndef SETTING_H
#define SETTING_H

#include <QSettings>
#include <QSharedPointer>
#include <QTextCodec>
#include <QCoreApplication>
#include <QFile>

class  IniSettings
{
public:
    QSettings  *iniFile;

public:
    IniSettings(const QString &path);
    ~IniSettings();

	void setValue(const QString &section, const QString &key, const QString &value);
	void removeNode(const QString &section);
	void removeKey(const QString &section, const QString &key);
	QString getValue(const QString &section, const QString &key);
};

#endif // SETTING_H
