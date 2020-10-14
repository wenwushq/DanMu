#include "DanMu.h"
#include <QTimer>
#include "DanMuControl.h"

DanMu::DanMu(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	m_strlstDanMus << QStringLiteral("弹幕测试一")
		<< QStringLiteral("弹幕测试增加长度二")
		<< QStringLiteral("弹幕三")
		<< QStringLiteral("弹幕测试四弹幕测试四弹幕测试四")
		<< QStringLiteral("弹幕测试五五五五")
		<< QStringLiteral("弹幕测试六")
		<< QStringLiteral("弹幕测试七七七七七七七七")
		<< QStringLiteral("测试八")
		<< QStringLiteral("弹幕测试九九九九长度")
		<< QStringLiteral("弹幕测试十十十");

	m_strColors << "color: rgb(170, 0, 0);"
		<< "color: rgb(0, 85, 0);"
		<< "color: rgb(170, 85, 0);"
		<< "color: rgb(170, 170, 0);"
		<< "color: rgb(170, 0, 255);"
		<< "color: rgb(85, 255, 0);";

	m_pDanMuCtrl = new DanMuControl();

	QTimer::singleShot(200, this, SLOT(slotGenAnimation()));
}

DanMu::~DanMu()
{
	if (m_pDanMuCtrl)
	{
		delete m_pDanMuCtrl;
		m_pDanMuCtrl = nullptr;
	}
}

//生成动画调用
void DanMu::slotGenAnimation()
{
	int nIndex = 0;
	for each (QString strDanMu in m_strlstDanMus)
	{
		QLabel *pLabel = new QLabel(strDanMu, this);
		nIndex = nIndex >= m_strColors.count() ? 0 : nIndex;
		pLabel->setStyleSheet(m_strColors[nIndex]);
		nIndex++;
		pLabel->move(0, -500);
		pLabel->setVisible(true);
		pLabel->setScaledContents(true);

		m_lstLabels << pLabel;
	}

	m_pDanMuCtrl->SetShowArea(0, 0, QSize(width(), height() / 2));//设置显示区域
	m_pDanMuCtrl->SetRowNumber(3);//设置显示几行弹幕
	m_pDanMuCtrl->SetLabels(m_lstLabels);
	m_pDanMuCtrl->Start();
}