//
// Created by Administrator on 25-12-7.
//

#include "ModelManager.h"

ModelManager& ModelManager::getInstance()
{
    static ModelManager instance;
    return instance;
}

bool ModelManager::initDetModel(const QString& detModelPath, const QString& detParamPath, const HTuple& deviceHandle)
{
    if (!m_detModelLoaded)
    {
        m_detModel.Init(detModelPath.toStdString(), detParamPath.toStdString(), deviceHandle);
        m_detModelLoaded = true;
    }
    return true;
}

bool ModelManager::initOCRModel(const QString& ocrDet1Path, const QString& ocrDet2Path, const QString& ocrRecog2Path, const HTuple& deviceHandle)
{
    if (!m_ocrModelLoaded)
    {
        m_ocrModel.Init(ocrDet1Path.toStdString(), ocrDet2Path.toStdString(), ocrRecog2Path.toStdString(), deviceHandle);
        m_ocrModelLoaded = true;
    }
    return true;
}

DetRes ModelManager::detInfer(const HObject& image)
{
    QMutexLocker locker(&m_detMutex);
    return m_detModel.Infer(image);
}

std::vector<ocrInfo> ModelManager::ocrInfer(const HObject& image,const  QString &type)
{
    QMutexLocker locker(&m_ocrMutex);
    return m_ocrModel.Infer(image,type);
}

void ModelManager::cleanup()
{
    QMutexLocker locker1(&m_detMutex);
    QMutexLocker locker2(&m_ocrMutex);
    // Halcon 模型会自动清理，这里只需要重置状态
    m_detModelLoaded = false;
    m_ocrModelLoaded = false;
}
