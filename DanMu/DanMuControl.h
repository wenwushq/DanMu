#pragma once

#include <QObject>
#include <QSize>
#include <QWidget>
#include <QMap>
#include <QPropertyAnimation>

class QGraphicsOpacityEffect;
class DanMuControl : public QObject
{
	Q_OBJECT

public:
	DanMuControl(QObject *parent = nullptr);
	~DanMuControl();

	//设置显示区域
	void SetShowArea(int nLeft, int nTop, QSize sizeArea);

	//设置显示几行弹幕
	void SetRowNumber(int nNumber);

	//设置弹幕标签
	void SetWidgets(QList<QWidget*> lstWidgets);

	void Start();//开始弹幕

private:
	void CalcPerRowYPos();//计算出每一行的起始Y坐标
	void GenAnimationByIndex(int nIndex);//生成动画
	void GenAnimationByHeight(int nHeight);//生成动画
	int GetWidgetInRowHeight(QWidget* pWidget);//根据widget的高度得到所在的哪一行

private slots:
	void slotGeometryAnimationValueChanged(const QVariant &varValue);//移动的动画值更改
	void slotOpacityAnimationFinished();//透明的动画完成
	void slotStartDanMu();//开始弹幕后计时开始下一条弹幕

private:
	bool m_bLoop;//是否循环
	int m_nShowAreaLeft, m_nShowAreaTop;//显示区域左上角点
	QSize m_sizeShowArea;//显示区域
	int m_nRowNumber;//显示几行弹幕
	QList<QWidget*> m_lstWidgets;

	QList<int> m_lstnRowYPos;//每一行的起始Y坐标
	int m_nDanMuSpace;//同一行两个弹幕间的间隙
	QMap<QWidget*, QPropertyAnimation*> m_mapWidgetOpacityAnim;
	QMap<QWidget*, QPropertyAnimation*> m_mapWidgetGeometryAnim;
	QMap<QPropertyAnimation*, QGraphicsOpacityEffect*> m_mapOpacityAnimEffect;
	int m_nDanMuIndex;//当前弹幕的索引
	bool m_bStartSuccess;
	QList<QPropertyAnimation*> m_lstpGeometryAnimationAfterGen;//在当前动画之后已经生成过动画的
};
