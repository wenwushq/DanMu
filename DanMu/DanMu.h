#pragma once

#include <QtWidgets/QWidget>
#include "ui_DanMu.h"

class DanMuControl;

class DanMu : public QWidget
{
    Q_OBJECT

public:
    DanMu(QWidget *parent = Q_NULLPTR);
	~DanMu();

private slots:
	void slotGenAnimation();//生成动画调用

private:
    Ui::DanMuClass ui;
	QStringList m_strlstDanMus;
	QStringList m_strColors;

	DanMuControl *m_pDanMuCtrl;
};
