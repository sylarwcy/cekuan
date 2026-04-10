#include "settings.h"


IniSettings::IniSettings(const QString &path)
{
    iniFile=new QSettings(path,QSettings::IniFormat);
    iniFile->setIniCodec(QTextCodec::codecForLocale());//QTextCodec::codecForName("System"));
    // 检查状态
    if (iniFile->status() != QSettings::NoError) {
        qCritical() << "QSettings 状态错误:" << iniFile->status();
        return;
    }
}
IniSettings::~IniSettings()
{
    delete iniFile;
}

void IniSettings::setValue(const QString &section, const QString &key, const QString &value)
{
    iniFile->beginGroup(section);
    iniFile->setValue(key, value);
    iniFile->endGroup();
}

void IniSettings::removeNode(const QString &section)
{
    iniFile->remove(section);
}

void IniSettings::removeKey(const QString &section, const QString &key)
{
    iniFile->beginGroup(section);
    iniFile->remove(key);
    iniFile->endGroup();
}

QString IniSettings::getValue(const QString &section, const QString &key){
	return iniFile->value(section+"/"+key).toString();
}