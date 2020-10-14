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
void DanMuControl::SetWidgets(QList<QWidget*> lstWidgets)
{
	m_lstWidgets << lstWidgets;

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
	if (nIndex >= m_lstWidgets.count())
		return;

	int nRowIndex = nIndex % m_lstnRowYPos.count();//计算出应该在哪一行
	int nHeight = m_lstnRowYPos[nRowIndex];
	GenAnimationByHeight(nHeight);
}

//生成动画
void DanMuControl::GenAnimationByHeight(int nHeight)
{
	if (m_nDanMuIndex >= m_lstWidgets.count())
		return;

	QWidget* pWidget = m_lstWidgets[m_nDanMuIndex];
	pWidget->setVisible(true);
	
	QRect rcWidget = pWidget->geometry();

	//设置标签移动到窗口的最右边
	pWidget->setGeometry(m_nShowAreaLeft + m_sizeShowArea.width(), nHeight - rcWidget.height() / 2, rcWidget.width(), rcWidget.height());

	//透明动画
	QGraphicsOpacityEffect* pOpacityEffect = new QGraphicsOpacityEffect(pWidget);
	pWidget->setGraphicsEffect(pOpacityEffect);
	pOpacityEffect->setOpacity(0);

	//由于透明动画更改属性的方式只能顶层窗口起作用，所以加了上面的代码
	QPropertyAnimation* pOpacityAnimation = new QPropertyAnimation(pOpacityEffect, "opacity");
	pOpacityAnimation->setDuration(3000);
	pOpacityAnimation->setStartValue(0);
	pOpacityAnimation->setEndValue(1);

	//从右向左移动动画
	QPropertyAnimation* pGeometryAnimation = new QPropertyAnimation(pWidget, "geometry");
	pGeometryAnimation->setDuration(m_sizeShowArea.width() / 300.0 * 6000);//根据显示区域宽度计算出移动时间
	pGeometryAnimation->setStartValue(pWidget->geometry());
	QRect rcWidgetEnd = pWidget->geometry();
	//pGeometryAnimation->setEndValue(QRect(m_nShowAreaLeft - rcWidgetEnd.width(), rcWidgetEnd.top(), rcWidgetEnd.width(), rcWidgetEnd.height()));
	pGeometryAnimation->setEndValue(QRect(m_nShowAreaLeft - 500, rcWidgetEnd.top(), rcWidgetEnd.width(), rcWidgetEnd.height()));//500是让Widget左侧一致，不然运动不一致，有快有慢
	pGeometryAnimation->setEasingCurve(QEasingCurve::Linear);

	m_mapWidgetOpacityAnim[pWidget] = pOpacityAnimation;
	m_mapWidgetGeometryAnim[pWidget] = pGeometryAnimation;
	m_mapOpacityAnimEffect[pOpacityAnimation] = pOpacityEffect;

	connect(pGeometryAnimation, SIGNAL(valueChanged(const QVariant&)), this, SLOT(slotGeometryAnimationValueChanged(const QVariant&)));
	connect(pOpacityAnimation, SIGNAL(finished()), this, SLOT(slotOpacityAnimationFinished()));

	pGeometryAnimation->start();
	pOpacityAnimation->start();
	m_nDanMuIndex++;
	if (m_nDanMuIndex >= m_lstWidgets.count())
		m_nDanMuIndex = 0;
}

//开始弹幕
void DanMuControl::Start()
{
	if (m_lstWidgets.isEmpty())
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
	
	QWidget *pWidgetCur = nullptr;
	QMapIterator<QWidget*, QPropertyAnimation*> iter(m_mapWidgetGeometryAnim);
	while (iter.hasNext()) {
		iter.next();
		if (iter.value() == pGeometryAnimation)
		{
			int nIndex = m_lstWidgets.indexOf(iter.key());
			pWidgetCur = m_lstWidgets[nIndex];
			break;
		}
	}
	
	QRect rcValue = varValue.toRect();
	if (pGeometryAnimation->startValue().toRect().left() - rcValue.right() >= m_nDanMuSpace)
	{
		if (!m_lstpGeometryAnimationAfterGen.contains(pGeometryAnimation) && !m_mapWidgetGeometryAnim.contains(m_lstWidgets[m_nDanMuIndex]))
		{//显示下一条弹幕
			GenAnimationByHeight(GetWidgetInRowHeight(pWidgetCur));
			m_lstpGeometryAnimationAfterGen << pGeometryAnimation;
		}
		else if (rcValue.left() <= m_nShowAreaLeft + 100)
		{//到左边，渐隐
			if (m_mapWidgetOpacityAnim[pWidgetCur]->state() != QAbstractAnimation::Running)
			{
				m_mapWidgetOpacityAnim[pWidgetCur]->setDirection(QAbstractAnimation::Backward);
				m_mapWidgetOpacityAnim[pWidgetCur]->start();
			}
		}
	}
}

//根据widget的高度得到所在的哪一行
int DanMuControl::GetWidgetInRowHeight(QWidget* pWidget)
{
	QRect rcValue = pWidget->geometry();
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

	QWidget *pWidgetCur = nullptr;
	QMapIterator<QWidget*, QPropertyAnimation*> iter(m_mapWidgetOpacityAnim);
	while (iter.hasNext()) {
		iter.next(); 
		if (iter.value() == pOpacityAnimation)
		{
			int nIndex = m_lstWidgets.indexOf(iter.key());
			pWidgetCur = m_lstWidgets[nIndex];
			break;
		}
	}

	if (pOpacityAnimation->direction() == QAbstractAnimation::Backward)
	{
		pWidgetCur->setVisible(false);

		m_lstpGeometryAnimationAfterGen.removeAll(m_mapWidgetGeometryAnim[pWidgetCur]);
		delete m_mapWidgetGeometryAnim[pWidgetCur];
		m_mapWidgetGeometryAnim.remove(pWidgetCur);
		delete m_mapOpacityAnimEffect[m_mapWidgetOpacityAnim[pWidgetCur]];
		m_mapOpacityAnimEffect.remove(m_mapWidgetOpacityAnim[pWidgetCur]);
		delete m_mapWidgetOpacityAnim[pWidgetCur];
		m_mapWidgetOpacityAnim.remove(pWidgetCur);
	}
}
