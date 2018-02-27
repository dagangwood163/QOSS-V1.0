#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QVTKWidget.h>
#include <QAction>
#include <QTreeWidget>
#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QstatusBar>
#include <QTreeWidgetItem>
#include <QButtonGroup>
#include <QRadioButton>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

#include "MyData.h"

#include "Qt/include/ParaboloidWidget.h"
#include "Qt/include/RestrictionWidget.h"
#include "Qt/include/GaussianWidget.h"
#include "Qt/include/CalculationWidget.h"

using namespace userInterface;
class mainWindow : public QMainWindow
{
	Q_OBJECT
	enum QVariantType
	{	
		FIELD = 10,
		
	};

public:
	mainWindow(QWidget *parent = 0);
	~mainWindow();

private:
	void init();

	//------------------- ��������------------------- 
	void createActions();
	void createMenus(); // �˵�
	void createToolBars();
	void createStatusBar();

	void createTreeWidgetItem(); // ����tree
	void createRightMenu(); // �Ҽ��˵�
	void createDetails(); //zuojian

	void createProject();

	// 
	void isNeedSave();

	void updateVtk();

	void updateLight();

	// �ı�3D��ʾ����
	void showDetails(int);


private slots:

	// �˵���Ӧ����
	void openFile();
	void newFile();
	void on_isShowBox();
	void on_isTransparentMirror();
	void on_isShowMirror();

	void on_modifyingRestriction();
	void on_delRestriction();

	void on_modifyingMirror();
	void on_modifyParameters();

	void on_restriction();
	void toReceiveRestriction(int);

	void createGaussian();
	void toReceiveGaussian(int);

	void on_createParaboloid();
	void toReceiveParaboloid(int);

	void on_PVVA();


	// ------------------- �Ҽ����� ----------------------------------
	void on_treeWidget_ContextMenuRequested(QPoint pos);// �Ҽ��˵�

	// ------------------- ������� ----------------------------------
	void on_treeWidget_leftPressed(QTreeWidgetItem *item, int column);
	void on_Details_FieldClicked(); //details Field 


private:
	vtkSmartPointer<vtkOrientationMarkerWidget> widget1;
	QVTKWidget widget; // vtk ��ʾ����
	vtkSmartPointer<vtkRenderWindowInteractor> interactor; // ����
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkAxesActor> axes; // ����

	QDockWidget * leftWidget; //����treeWidget�Ĵ���
	QTreeWidget * treeWidget;

	QDockWidget * detailsDockWidget;
	QWidget * detailsWidget;

	QLabel * locationLabel;  // ״̬����ǩ
	QPushButton * unitBtn;   // ��λ��ť

	//----------- Menu ----------------- 
	QMenu * fileMenu;  //�˵���
	QMenu * viewMenu;  //��ʾ��
	QMenu * eidtMenu;  //�༭��
	QMenu * ModelMenu;  //ģ����
	QMenu * SourceMenu;  //Դ��
	QMenu * CalMenu;  //������

	//�Ҽ��˵�
	QMenu *R_Tree_MirrorTypeMenu;
	QMenu *R_Tree_MirrorParMenu;
	QMenu *R_Tree_RestrictionMenu;
	QMenu *R_BlankMenu;

	//----------- ToolBar ------------------- 
	QToolBar * fileTool;   //������
	QToolBar * constructTool;  //ģ����

	//----------- Action ----------------- 
	//�ļ��˵���
	QAction * saveFileAction;
	QAction * openFileAction;
	QAction * newFileAction;
	//�ļ��˵�--view
	QAction * viewAction;  // �ӽ�
	QLabel * viewLabel;
	QComboBox * viewComboBox;

	QAction * isShowBoxAction;

	QAction * GaussianAction;     //��˹��Դ
	QAction * PVVAAction;     //��˹��Դ

	// �Ҽ�
	QAction * modifyingMirrorAction;
	QAction * restrictionAction;
	QAction * modifyingRestrictionAction;
	QAction * delRestrictionAction;

	QAction * modifyParametersAction;
	QAction * isShowMirrorAction;
	QAction * isTransparentAction;

	//------------- TreeWidgetItem------------------- 
	QTreeWidgetItem * definitionsTreeItem;
	QTreeWidgetItem * variablesTreeItem;
	QTreeWidgetItem * modelTreeItem;
	QTreeWidgetItem * geometryTreeItem;
	QTreeWidgetItem * sourceTreeItem;
	QTreeWidgetItem * soucreFieldTreeItem;
	QTreeWidgetItem * lightTreeItem;
	QTreeWidgetItem * fieldTreeItem;

	QTreeWidgetItem * rightSelectItem;
	QTreeWidgetItem * leftSelectItem; // �Ҽ�ѡ�еĽڵ�

	//****** Details********
	QButtonGroup * dimensionGroupBtn;
	QRadioButton * ThreeDBtn;
	QRadioButton * TwoDBtn;
	QButtonGroup * fieldGroupBtn;
	QRadioButton * ExBtn;
	QRadioButton * EyBtn;
	QRadioButton * EzBtn;
	QRadioButton * HxBtn;
	QRadioButton * HyBtn;
	QRadioButton * HzBtn;
	QButtonGroup * pmGroupBtn;
	QRadioButton * magnitudeBtn;
	QRadioButton * phaseBtn;
	QButtonGroup * powerGroupBtn;
	QRadioButton * linearBtn;
	QRadioButton * dbBtn;

	vector<QTreeWidgetItem*> mirrorTreeWidgetItem;

	MyData * myData;
	Mirror * tempMirror;
	Restriction * tempRestriction;

	// ����ָ��
	ParaboloidWidget * paraboloidWidget;
	RestrictionWidget * restrictionWidget;
	GaussianWidget * gaussianWidget;

	bool isExistenceOpenWin;
	bool isNew;
	int fieldNum;

};

#endif // MAINWINDOW_H