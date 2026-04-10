//
// Created by Administrator on 25-12-7.
//

#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "halcon_inc.h"
#include "imgAI.h"
#include <QMutex>
#include <QSharedPointer>

class ModelManager : public QObject {
    Q_OBJECT

public:
    static ModelManager& getInstance();

    bool initDetModel(const QString& detModelPath, const QString& detParamPath, const HTuple& deviceHandle);
    bool initOCRModel(const QString& ocrDet1Path, const QString& ocrDet2Path, const QString& ocrRecog2Path, const HTuple& deviceHandle);

    DetRes detInfer(const HObject& image);
    std::vector<ocrInfo> ocrInfer(const HObject& image,const  QString &type);

    // bool isDetModelLoaded() const { return m_detModelLoaded; }
    // bool isOCRModelLoaded() const { return m_ocrModelLoaded; }

    void cleanup();

private:
    ModelManager() = default;
    ~ModelManager() = default;

    HalconDet m_detModel;
    HalconOCR m_ocrModel;
    QMutex m_detMutex;
    QMutex m_ocrMutex;
    bool m_detModelLoaded = false;
    bool m_ocrModelLoaded = false;
};



#endif //MODELMANAGER_H
