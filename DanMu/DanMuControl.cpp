#include "DanMuControl.h"
#include <QGraphicsOpacityEffect>
#include <QTimer>

DanMuControl::DanMuControl(QObject *parent)
	: QObject(parent)
{
	m_bLoop = true;
	m_nShowAreaLeft = 0;
	m_nShowAreaTop = 0;
	m_sizeShowArea = QSize(0, 0);
	m_nRowNumber = 0;
	m_nDanMuSpace = 50;
	m_nDanMuIndex = 0;
	m_bStartSuccess = false;
}

DanMuControl::~DanMuControl()
{
}

//设置显示区域
void DanMuControl::SetShowArea(int nLeft, int nTop, QSize sizeArea)
{
	m_nShowAreaLeft = nLeft;
	m_nShowAreaTop = nTop;
	m_sizeShowArea = sizeArea;
}

//设置显示几行弹幕
void DanMuControl::SetRowNumber(int nNumber)
{
	m_nRowNumber = nNumber;

	if (m_lstnRowYPos.isEmpty())
		CalcPerRowYPos();//计算出每一行的起始Y坐标
}

//设置弹幕标签
void DanMuControl::SetLabels(QList<QLabel*> lstLabels)
{
	m_lstLabels << lstLabels;

}

//计算出每一行的起始Y坐标
void DanMuControl::CalcPerRowYPos()
{
	double d1RowHeight = m_sizeShowArea.height() / (double)m_nRowNumber;
	
	for(int nIndex = 0; nIndex < m_nRowNumber; nIndex ++)
		m_lstnRowYPos << m_nShowAreaTop + d1RowHeight  / 2 + d1RowHeight * nIndex;
}

//生成动画
void DanMuControl::GenAnimationByIndex(int nIndex)
{
	if (nIndex >= m_lstLabels.count())
		return;

	int nRowIndex = nIndex % m_lstnRowYPos.count();//计算出应该在哪一行
	int nHeight = m_lstnRowYPos[nRowIndex];
	GenAnimationByHeight(nHeight);
}

//生成动画
void DanMuControl::GenAnimationByHeight(int nHeight)
{
	if (m_nDanMuIndex >= m_lstLabels.count())
		return;

	QLabel* pLabel = m_lstLabels[m_nDanMuIndex];
	pLabel->setScaledContents(true);
	pLabel->setVisible(true);
	
	QRect rcLabel = pLabel->geometry();

	//设置标签移动到窗口的最右边
	pLabel->setGeometry(m_nShowAreaLeft + m_sizeShowArea.width(), nHeight - rcLabel.height() / 2, rcLabel.width(), rcLabel.height());

	//透明动画
	QGraphicsOpacityEffect* pOpacityEffect = new QGraphicsOpacityEffect(pLabel);
	pLabel->setGraphicsEffect(pOpacityEffect);
	pOpacityEffect->setOpacity(0);

	//由于透明动画更改属性的方式只能顶层窗口起作用，所以加了上面的代码
	QPropertyAnimation* pOpacityAnimation = new QPropertyAnimation(pOpacityEffect, "opacity");
	pOpacityAnimation->setDuration(3000);
	pOpacityAnimation->setStartValue(0);
	pOpacityAnimation->setEndValue(1);

	//从右向左移动动画
	QPropertyAnimation* pGeometryAnimation = new QPropertyAnimation(pLabel, "geometry");
	pGeometryAnimation->setDuration(m_sizeShowArea.width() / 300.0 * 6000);//根据显示区域宽度计算出移动时间
	pGeometryAnimation->setStartValue(pLabel->geometry());
	QRect rcLabelEnd = pLabel->geometry();
	//pGeometryAnimation->setEndValue(QRect(m_nShowAreaLeft - rcLabelEnd.width(), rcLabelEnd.top(), rcLabelEnd.width(), rcLabelEnd.height()));
	pGeometryAnimation->setEndValue(QRect(m_nShowAreaLeft - 500, rcLabelEnd.top(), rcLabelEnd.width(), rcLabelEnd.height()));//500是让label左侧一致，不然运动不一致，有快有慢
	pGeometryAnimation->setEasingCurve(QEasingCurve::Linear);

	m_mapLabelOpacityAnim[pLabel] = pOpacityAnimation;
	m_mapLabelGeometryAnim[pLabel] = pGeometryAnimation;

	connect(pGeometryAnimation, SIGNAL(valueChanged(const QVariant&)), this, SLOT(slotGeometryAnimationValueChanged(const QVariant&)));
	connect(pOpacityAnimation, SIGNAL(finished()), this, SLOT(slotOpacityAnimationFinished()));

	pGeometryAnimation->start();
	pOpacityAnimation->start();
	m_nDanMuIndex++;
	if (m_nDanMuIndex >= m_lstLabels.count())
		m_nDanMuIndex = 0;
}

//开始弹幕
void DanMuControl::Start()
{
	if (m_lstLabels.isEmpty())
		return;

	m_nDanMuIndex = 0;
	m_bStartSuccess = false;
	GenAnimationByIndex(m_nDanMuIndex);

	QTimer::singleShot(500, this, SLOT(slotStartDanMu()));
}

//开始弹幕后计时开始下一条弹幕
void DanMuControl::slotStartDanMu()
{
	GenAnimationByIndex(m_nDanMuIndex);

	if(m_nDanMuIndex != m_nRowNumber)
		QTimer::singleShot(500, this, SLOT(slotStartDanMu()));
	else
		m_bStartSuccess = true;
}

//移动的动画值更改
void DanMuControl::slotGeometryAnimationValueChanged(const QVariant &varValue)
{
	if (!m_bStartSuccess)
		return;

	QPropertyAnimation *pGeometryAnimation = qobject_cast<QPropertyAnimation*>(sender());
	if (!pGeometryAnimation)
		return;
	
	QLabel *pLabelCur = nullptr;
	QMapIterator<QLabel*, QPropertyAnimation*> iter(m_mapLabelGeometryAnim);
	while (iter.hasNext()) {
		iter.next();
		if (iter.value() == pGeometryAnimation)
		{
			int nIndex = m_lstLabels.indexOf(iter.key());
			pLabelCur = m_lstLabels[nIndex];
			break;
		}
	}
	
	QRect rcValue = varValue.toRect();
	if (pGeometryAnimation->startValue().toRect().left() - rcValue.right() >= m_nDanMuSpace)
	{
		if (!m_lstpGeometryAnimationAfterGen.contains(pGeometryAnimation) && !m_mapLabelGeometryAnim.contains(m_lstLabels[m_nDanMuIndex]))
		{//显示下一条弹幕
			GenAnimationByHeight(GetLabelInRowHeight(pLabelCur));
			m_lstpGeometryAnimationAfterGen << pGeometryAnimation;
		}
		else if (rcValue.left() <= m_nShowAreaLeft + 100)
		{//到左边，渐隐
			if (m_mapLabelOpacityAnim[pLabelCur]->state() != QAbstractAnimation::Running)
			{
				m_mapLabelOpacityAnim[pLabelCur]->setDirection(QAbstractAnimation::Backward);
				m_mapLabelOpacityAnim[pLabelCur]->start();
			}
		}
	}
}

//根据label的高度得到所在的哪一行
int DanMuControl::GetLabelInRowHeight(QLabel* pLabel)
{
	QRect rcValue = pLabel->geometry();
	int nHeight = rcValue.top() + rcValue.height() / 2;
	for each (int nYPos in m_lstnRowYPos)
	{
		if (qAbs(nHeight - nYPos) <= 2)
		{
			nHeight = nYPos;
			break;
		}
	}
	return nHeight;
}

//透明的动画完成
void DanMuControl::slotOpacityAnimationFinished()
{
	QPropertyAnimation *pOpacityAnimation = qobject_cast<QPropertyAnimation*>(sender());
	if (!pOpacityAnimation)
		return;

	QLabel *pLabelCur = nullptr;
	QMapIterator<QLabel*, QPropertyAnimation*> iter(m_mapLabelOpacityAnim);
	while (iter.hasNext()) {
		iter.next();
		if (iter.value() == pOpacityAnimation)
		{
			int nIndex = m_lstLabels.indexOf(iter.key());
			pLabelCur = m_lstLabels[nIndex];
			break;
		}
	}

	if (pOpacityAnimation->direction() == QAbstractAnimation::Backward)
	{
		pLabelCur->setVisible(false);

		m_lstpGeometryAnimationAfterGen.removeAll(m_mapLabelGeometryAnim[pLabelCur]);
		delete m_mapLabelGeometryAnim[pLabelCur];
		m_mapLabelGeometryAnim.remove(pLabelCur);
		delete m_mapLabelOpacityAnim[pLabelCur];
		m_mapLabelOpacityAnim.remove(pLabelCur);
	}
}