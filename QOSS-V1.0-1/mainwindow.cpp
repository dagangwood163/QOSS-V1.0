#include "mainwindow.h"
#include <Qt/include/PreDialog.h>
#include "Qt/include/ModelWizard.h"
#include "Qt/include/MirrorTypeWidget.h"

#include "VTK/include/Mirror.h"
#include "VTK/include/Restriction.h"
#include "VTK/include/MirrorFactory.h"
#include "VTK/include/LimitBox.h"
#include "VTK/include/LightShow.h"
#include "VTK/include/Radiator.h"

#include "util/Definition.h"

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindow.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkOrientationMarkerWidget.h>
#include <QApplication>

#include <QMessageBox>


using namespace userInterface;

mainWindow::mainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setCentralWidget(&widget);
	resize(1200, 800);

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(1.0, 1.0, 1.0);

	auto window = widget.GetRenderWindow();
	window->AddRenderer(renderer);

	PreDialog preDialog;
	if (preDialog.exec() != QDialog::Accepted)
	{
		exit(1);
	}
	myData = MyData::getInstance();

	//����Ĭ�Ϸ�����
	myData->createRadiator();
	renderer->AddActor(myData->getRadiator()->getActorModel());
	renderer->AddActor(myData->getRadiator()->getActorRay());

	// ����Ĭ�ϵľ���
	myData->createDefaultMirror();
	//for (int i = 0; i < myData->getNumOfMirrors(); ++i)
	for (int i = 0; i < 3; ++i)
	{
		renderer->AddActor(myData->getMirrorByNum(i)->getActor());
	}

	// �������ƺ���
	renderer->AddActor(myData->getLimitBox()->getActor());

	// ����Ĭ�ϵĹ���
	myData->createDefaultLigthShow();
	std::list<vtkSmartPointer<vtkActor>> tempActors = 
		myData->getDefaultLightShow()->getActors();
	for (auto& x : tempActors)
		renderer->AddActor(x);

	
	double axesScale = myData->getLimitBox()->getMaxSize();
	// ��ʼ��vtk����
	axes = vtkSmartPointer<vtkAxesActor>::New();
	axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1, 0, 0);//�޸�X������ɫΪ��ɫ  
	axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0, 2, 0);//�޸�Y������ɫΪ��ɫ  
	axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0, 0, 3);//�޸�Z������ɫΪ��ɫ  
	axes->SetConeRadius(0.3);
	axes->SetConeResolution(20);
	//axes->SetTotalLength(10, 10, 10); //�޸�����ߴ�
	
	interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(window);

	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);
	interactor->Initialize();

	vtkCamera *aCamera = vtkCamera::New();
	aCamera->SetViewUp(0, 0, 1);//���ӽ�λ�� 
	aCamera->SetPosition(0, -3 * axesScale, 0);//��۲����λ
	aCamera->SetFocalPoint(0, 0, 0);//�轹�� 
	aCamera->ComputeViewPlaneNormal();//�Զ�
	renderer->SetActiveCamera(aCamera);

	renderer->ResetCamera();
	window->Render();

	widget1 = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
	widget1->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget1->SetOrientationMarker(axes);
	widget1->SetInteractor(interactor);
	widget1->SetViewport(0.0, 0.0, 0.25, 0.25);
	widget1->SetEnabled(1);
	widget1->InteractiveOff();

	// ����dock����
	leftWidget = new QDockWidget("Navigation", this);
	leftWidget->setFeatures(QDockWidget::DockWidgetMovable);
	leftWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//leftWidget->setWindowFlags(Qt::FramelessWindowHint);
	addDockWidget(Qt::LeftDockWidgetArea, leftWidget);

	// treeWidget
	treeWidget = new QTreeWidget;
	treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(on_treeWidget_ContextMenuRequested(QPoint)));
	connect(treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
		this, SLOT(on_treeWidget_leftPressed(QTreeWidgetItem*, int)));
	treeWidget->setHeaderHidden(true);  // ���ر�ͷ
	leftWidget->setWidget(treeWidget);

	//RightWidget
	detailsDockWidget = new QDockWidget("Details", this);
	detailsDockWidget->setFeatures(QDockWidget::DockWidgetMovable);
	detailsDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
	detailsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//leftWidget->setWindowFlags(Qt::FramelessWindowHint);
	addDockWidget(Qt::LeftDockWidgetArea, detailsDockWidget);

	createTreeWidgetItem();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createRightMenu();
	createDetails();
	//creareWindows();

	init();
}

mainWindow::~mainWindow()
{

}

void mainWindow::init()
{
	isExistenceOpenWin = false;
	isNew = true;
	fieldNum = 1;
}

void mainWindow::createActions()
{
	//**************�˵�*****************
	//����
	saveFileAction = new QAction(QIcon(tr("Qt/images/save.png")), tr("Save"), this);
	saveFileAction->setShortcut(tr("Ctrl +��s"));
	saveFileAction->setStatusTip(tr("Save a file"));
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	//��
	openFileAction = new QAction(QIcon(tr("Qt/images/open.png")), tr("Open"), this);
	openFileAction->setShortcut(tr("Ctrl +��O"));
	openFileAction->setStatusTip(tr("Open a file"));
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	//�½�
	newFileAction = new QAction(QIcon(tr("Qt/images/new.png")), tr("New"), this);
	newFileAction->setShortcut(tr("Ctrl +��N"));
	newFileAction->setStatusTip(tr("New a file"));
	connect(newFileAction, SIGNAL(triggered()), this, SLOT(newFile()));

	// view ��ͼ

	//��ʼ���ӽ�
	viewAction = new QAction(QIcon(tr("Qt/images/view.png")), tr("View"), this);
	viewAction->setStatusTip(tr("Initialize the view"));
	connect(viewAction, SIGNAL(triggered()), this, SLOT(viewInitFile()));

	//�޸��ӽ�
	viewLabel = new QLabel(tr("View"));
	viewComboBox = new QComboBox;
	viewComboBox->addItem("View XZ plane(original)");
	viewComboBox->addItem("View YZ plane");
	viewComboBox->addItem("View XY plane");
	viewComboBox->addItem("View -YZ plane");
	viewComboBox->addItem("View -XZ plane");
	viewComboBox->addItem("View -XY plane");
	connect(viewComboBox, SIGNAL(activated(int)), this, SLOT(setView(int)));

	isShowBoxAction = new QAction(tr("Box"), this);
	isShowBoxAction->setStatusTip(tr("is show Box"));
	isShowBoxAction->setCheckable(true);
	isShowBoxAction->setChecked(true);
	connect(isShowBoxAction, SIGNAL(triggered()), this, SLOT(on_isShowBox()));

	//��˹Դ
	GaussianAction = new QAction(QIcon(tr("Qt/images/Gaussian.png")), tr("Gaussian"), this);
	GaussianAction->setStatusTip(tr("Create a Gaussian source"));
	connect(GaussianAction, SIGNAL(triggered()), this, SLOT(createGaussian()));

	// 
	PVVAAction = new QAction(QIcon(tr("Qt/images/PVVA.png")), tr("Fast calculation"),
		this);
	PVVAAction->setStatusTip(tr("Fast calculation by PVVA"));
	connect(PVVAAction, SIGNAL(triggered()), this, SLOT(on_PVVA()));
}

void mainWindow::createMenus()
{
	// �ļ��˵�
	//this->menuBar()
	fileMenu = this->menuBar()->addMenu(tr("Files"));

	fileMenu->addAction(saveFileAction);
	fileMenu->addAction(openFileAction);
	fileMenu->addAction(newFileAction);
	fileMenu->addSeparator();
	//fileMenu->addAction(LightSourceAction);

	viewMenu = this->menuBar()->addMenu(tr("View"));

	viewMenu->addAction(isShowBoxAction);

	eidtMenu = this->menuBar()->addMenu(tr("Edit"));
	ModelMenu = this->menuBar()->addMenu(tr("Model"));
	SourceMenu = this->menuBar()->addMenu(tr("Source"));
	CalMenu = this->menuBar()->addMenu(tr("simulation"));
}

void mainWindow::createToolBars()
{
	//file ��
	fileTool = addToolBar("Files");
	fileTool->addAction(saveFileAction);
	fileTool->addAction(openFileAction);
	fileTool->addAction(newFileAction);

	fileTool->addSeparator();
	fileTool->addAction(viewAction);
	fileTool->addSeparator();
	fileTool->addWidget(viewLabel);
	fileTool->addWidget(viewComboBox);
	fileTool->addSeparator();
	fileTool->addAction(GaussianAction);
	fileTool->addAction(PVVAAction);
}

void mainWindow::createStatusBar()
{
	locationLabel = new QLabel("    ");
	locationLabel->setAlignment(Qt::AlignHCenter);
	locationLabel->setMinimumSize(locationLabel->sizeHint());
	this->statusBar()->addWidget(locationLabel, 90);

	unitBtn = new QPushButton(tr("Unit: m"));
	connect(unitBtn, SIGNAL(clicked()), this, SLOT(changeUnit()));
	//connect(unitBtn, SIGNAL(clicked()), this, SLOT(changeUnit()));
	this->statusBar()->addWidget(unitBtn);
}

void mainWindow::createTreeWidgetItem()
{
	definitionsTreeItem = new QTreeWidgetItem(treeWidget, QStringList(QString("Definitions")));
	modelTreeItem = new QTreeWidgetItem(treeWidget, QStringList(QString("Model")));

	definitionsTreeItem->setExpanded(true);
	modelTreeItem->setExpanded(true);

	variablesTreeItem = new QTreeWidgetItem(QStringList(QString("Variables")));
	variablesTreeItem->setData(0, Qt::UserRole, QVariant(3));
	// ����3�Ǹ���־

	QTreeWidgetItem *child1;
	child1 = new QTreeWidgetItem;
	child1->setText(0, tr("eps0") + tr(" = ") +
		tr("8.85418781761e-12"));
	variablesTreeItem->addChild(child1);

	QTreeWidgetItem *child2;
	child2 = new QTreeWidgetItem;
	child2->setText(0, tr("Pi") + tr(" = ") +
		tr("3.14159265358979"));
	variablesTreeItem->addChild(child2);

	QTreeWidgetItem *child3;
	child3 = new QTreeWidgetItem;
	child3->setText(0, tr("mu0") + tr(" = ") +
		tr("Pi*4e-7"));
	variablesTreeItem->addChild(child3);

	QTreeWidgetItem *child4;
	child4 = new QTreeWidgetItem;
	child4->setText(0, tr("c0") + tr(" = ") +
		tr("1/sqrt(eps0*mu0)"));
	variablesTreeItem->addChild(child4);

	QTreeWidgetItem *child5;
	child5 = new QTreeWidgetItem;
	child5->setText(0, tr("zf0") + tr(" = ") +
		tr("sqrt(mu0/eps0))"));
	variablesTreeItem->addChild(child5);
	definitionsTreeItem->addChild(variablesTreeItem);
	variablesTreeItem->setExpanded(true);

	sourceTreeItem = new QTreeWidgetItem(QStringList(QString("Source")));
	modelTreeItem->addChild(sourceTreeItem);
	QTreeWidgetItem *childFre = new QTreeWidgetItem;
	childFre->setText(0, tr("Fre = ") + QString::number(myData->getFrequency()));
	sourceTreeItem->addChild(childFre);

	// ģʽ
	QTreeWidgetItem *childPar = new QTreeWidgetItem;
	// ������
	QTreeWidgetItem *childRadiator = new QTreeWidgetItem;

	if (0 == myData->getPattern())
	{
		childPar->setText(0, tr("Pattern: Lower order"));
		childRadiator->setText(0, tr("Vlasov Launcher"));
	}	
	else if (1 == myData->getPattern())
		childPar->setText(0, tr("Pattern: Higher order"));
	else
		childPar->setText(0, tr("Pattern: Waveguide"));


	sourceTreeItem->addChild(childPar);
	sourceTreeItem->addChild(childRadiator);


	sourceTreeItem->setExpanded(true);

	geometryTreeItem = new QTreeWidgetItem(QStringList(QString("Geometry")));
	modelTreeItem->addChild(geometryTreeItem);

	// ���ӵ�tree
	for (int i = 0; i < myData->getNumOfMirrors(); ++i)
	{
		QTreeWidgetItem *childMirror = new QTreeWidgetItem;
		childMirror->setText(0, tr("Mirror") + QString::number(i+1));
		childMirror->setData(0, Qt::UserRole, QVariant(0));
		childMirror->setData(1, Qt::UserRole, QVariant(i));
		geometryTreeItem->addChild(childMirror);
		childMirror->addChild(myData->getMirrorByNum(i)->getTree());
		childMirror->child(0)->setData(2, Qt::UserRole, QVariant(i));
		mirrorTreeWidgetItem.push_back(childMirror);
		childMirror->setExpanded(true);
	}

	// ���ӵ�tree
	QTreeWidgetItem *childBox = new QTreeWidgetItem;
	childBox->setText(0, tr("LimitBox"));
	geometryTreeItem->addChild(childBox);
	QTreeWidgetItem *childBoxX = new QTreeWidgetItem;
	childBoxX->setText(0, tr("X = ") + QString::number(myData->getLimitBox()->getLength()));
	childBox->addChild(childBoxX);
	QTreeWidgetItem *childBoxY = new QTreeWidgetItem;
	childBoxY->setText(0, tr("Y = ") + QString::number(myData->getLimitBox()->getHeight()));
	childBox->addChild(childBoxY);
	QTreeWidgetItem *childBoxZ = new QTreeWidgetItem;
	childBoxZ->setText(0, tr("Z = ") + QString::number(myData->getLimitBox()->getWidth()));
	childBox->addChild(childBoxZ);
	childBox->setExpanded(true);

	geometryTreeItem->setExpanded(true);

	lightTreeItem = new QTreeWidgetItem(QStringList(QString("Light")));
	modelTreeItem->addChild(lightTreeItem);

	fieldTreeItem = new QTreeWidgetItem(QStringList(QString("Field")));
	modelTreeItem->addChild(fieldTreeItem);
	soucreFieldTreeItem = new QTreeWidgetItem(QStringList(QString("SourceField")));
	soucreFieldTreeItem->setData(0, Qt::UserRole, QVariant(FIELD));
	soucreFieldTreeItem->setData(1, Qt::UserRole, QVariant(0));
	fieldTreeItem->addChild(soucreFieldTreeItem);

}

void mainWindow::createRightMenu()
{
	R_Tree_MirrorTypeMenu = new QMenu(this);
	R_BlankMenu = new QMenu(this);

	modifyingMirrorAction = new QAction(tr("Modifying the mirror type"), this);
	QFont font("Microsoft YaHei", 10, 75);
	modifyingMirrorAction->setFont(font);
	modifyingMirrorAction->setStatusTip(tr("Modifying the mirror type"));
	connect(modifyingMirrorAction, SIGNAL(triggered()), this, SLOT(on_modifyingMirror()));

	restrictionAction = new QAction(tr("Add restriction"), this);
	restrictionAction->setFont(font);
	restrictionAction->setStatusTip(tr("Add restriction"));
	connect(restrictionAction, SIGNAL(triggered()), this, SLOT(on_restriction()));

	isShowMirrorAction = new QAction(tr("Show"), this);
	isShowMirrorAction->setStatusTip(tr("isShow"));
	isShowMirrorAction->setCheckable(true);
	isShowMirrorAction->setChecked(true);
	connect(isShowMirrorAction, SIGNAL(triggered()), this, SLOT(on_isShowMirror()));

	isTransparentAction = new QAction(tr("Transparent"), this);
	isTransparentAction->setStatusTip(tr("isTransparent"));
	isTransparentAction->setCheckable(true);
	isTransparentAction->setChecked(false);
	connect(isTransparentAction, SIGNAL(triggered()), this, SLOT(on_isTransparentMirror()));

	R_Tree_MirrorTypeMenu->addAction(modifyingMirrorAction);
	R_Tree_MirrorTypeMenu->addAction(restrictionAction);
	R_Tree_MirrorTypeMenu->addAction(isShowMirrorAction);
	R_Tree_MirrorTypeMenu->addAction(isTransparentAction);

	R_Tree_MirrorParMenu = new QMenu(this);
	modifyParametersAction = new QAction(tr("Modifying parameters"), this);
	modifyParametersAction->setFont(font);
	modifyParametersAction->setStatusTip(tr("Modifying parameters"));
	connect(modifyParametersAction, SIGNAL(triggered()), this, SLOT(on_modifyParameters()));

	R_Tree_MirrorParMenu->addAction(modifyParametersAction);

	R_Tree_RestrictionMenu = new QMenu(this);

	modifyingRestrictionAction = new QAction(tr("Modifying Restriction"), this);
	modifyingRestrictionAction->setStatusTip(tr("Modifying Restriction"));
	connect(modifyingRestrictionAction, SIGNAL(triggered()), this, SLOT(on_modifyingRestriction()));

	delRestrictionAction = new QAction(tr("Delete Restriction"), this);
	delRestrictionAction->setStatusTip(tr("Delete Restriction"));
	connect(delRestrictionAction, SIGNAL(triggered()), this, SLOT(on_delRestriction()));

	R_Tree_RestrictionMenu->addAction(modifyingRestrictionAction);
	R_Tree_RestrictionMenu->addAction(delRestrictionAction);
}

void mainWindow::createDetails()
{
	dimensionGroupBtn = new QButtonGroup();
	ThreeDBtn = new QRadioButton(tr("3D"));
	TwoDBtn = new QRadioButton(tr("2D"));
	dimensionGroupBtn->addButton(ThreeDBtn, 0);
	dimensionGroupBtn->addButton(TwoDBtn, 1);
	connect(ThreeDBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(TwoDBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));

	QGridLayout * dimensionLayout = new QGridLayout;
	dimensionLayout->addWidget(ThreeDBtn, 0, 0);
	dimensionLayout->addWidget(TwoDBtn, 0, 1);

	QGroupBox * dimensionGroupBox = new QGroupBox;
	dimensionGroupBox->setTitle(tr("dimension"));
	dimensionGroupBox->setLayout(dimensionLayout);

	fieldGroupBtn = new QButtonGroup();
	ExBtn = new QRadioButton(tr("Ex"));
	EyBtn = new QRadioButton(tr("Ey"));
	EzBtn = new QRadioButton(tr("Ez"));
	HxBtn = new QRadioButton(tr("Hx"));
	HyBtn = new QRadioButton(tr("Hy"));
	HzBtn = new QRadioButton(tr("Hz"));

	fieldGroupBtn->addButton(ExBtn, 0);
	fieldGroupBtn->addButton(EyBtn, 1);
	fieldGroupBtn->addButton(EzBtn, 2);
	fieldGroupBtn->addButton(HxBtn, 3);
	fieldGroupBtn->addButton(HyBtn, 4);
	fieldGroupBtn->addButton(HzBtn, 5);

	connect(ExBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(EyBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(EzBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(HxBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(HyBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(HzBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));

	QGridLayout * fieldLayout = new QGridLayout;
	fieldLayout->addWidget(ExBtn, 0, 0);
	fieldLayout->addWidget(EyBtn, 1, 0);
	fieldLayout->addWidget(EzBtn, 2, 0);
	fieldLayout->addWidget(HxBtn, 0, 1);
	fieldLayout->addWidget(HyBtn, 1, 1);
	fieldLayout->addWidget(HzBtn, 2, 1);

	QGroupBox * fieldGroupBox = new QGroupBox;
	fieldGroupBox->setTitle(tr("field"));
	fieldGroupBox->setLayout(fieldLayout);

	// pmGroupBox
	pmGroupBtn = new QButtonGroup();
	magnitudeBtn = new QRadioButton(tr("magnitude"));
	phaseBtn = new QRadioButton(tr("phase"));

	pmGroupBtn->addButton(magnitudeBtn, 0);
	pmGroupBtn->addButton(phaseBtn, 1);

	connect(magnitudeBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(phaseBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));

	QGridLayout * pmLayout = new QGridLayout;
	pmLayout->addWidget(magnitudeBtn, 0, 0);
	pmLayout->addWidget(phaseBtn, 0, 1);

	QGroupBox * pmGroupBox = new QGroupBox;
	pmGroupBox->setTitle(tr("parameter"));
	pmGroupBox->setLayout(pmLayout);

	// pmGroupBox
	powerGroupBtn = new QButtonGroup();
	linearBtn = new QRadioButton(tr("linear"));
	dbBtn = new QRadioButton(tr("dB"));
	powerGroupBtn->addButton(dbBtn, 0);
	powerGroupBtn->addButton(linearBtn, 1);


	QGridLayout * powerLayout = new QGridLayout;
	powerLayout->addWidget(linearBtn, 0, 0);
	powerLayout->addWidget(dbBtn, 0, 1);

	connect(linearBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));
	connect(dbBtn, SIGNAL(clicked()), this, SLOT(on_Details_FieldClicked()));

	// powerGroupBox
	QGroupBox * powerGroupBox = new QGroupBox;
	powerGroupBox->setTitle(tr("power"));
	powerGroupBox->setLayout(powerLayout);

	QVBoxLayout * boxLayout = new QVBoxLayout;
	boxLayout->addWidget(dimensionGroupBox);
	boxLayout->addWidget(fieldGroupBox);
	boxLayout->addWidget(pmGroupBox);
	boxLayout->addWidget(powerGroupBox);

	detailsWidget = new QWidget;
	detailsWidget->setLayout(boxLayout);
	detailsDockWidget->setWidget(detailsWidget);
	detailsDockWidget->close();
}

void mainWindow::createProject()
{

}

void mainWindow::isNeedSave()
{
	MyData * mydata = MyData::getInstance();
	if (mydata->getModifiedFlag())
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "question", "The data has been changed. Is it saved?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (rb == QMessageBox::Yes)
		{
			QMessageBox::aboutQt(NULL, "About Qt");
		}
	}
}

// -------------------- slots ���� ------------------------------------

void mainWindow::openFile()
{
	isNeedSave();

}

void mainWindow::newFile()
{
	isNeedSave();
	ModelWizard modelWizard;
	if (modelWizard.exec() != QDialog::Accepted)
	{
		return;
	}

}

void mainWindow::on_isShowBox()
{
	//isShowBoxAction->isChecked();
	myData->getLimitBox()->setIsTransparent(isShowBoxAction->isChecked());

	updateVtk();
}

void mainWindow::on_isTransparentMirror()
{
	int index = rightSelectItem->data(1, Qt::UserRole).toInt();
	myData->getMirrorByNum(index)->switchIsTransparent();
	updateVtk();
}

void mainWindow::on_isShowMirror()
{
	int index = rightSelectItem->data(1, Qt::UserRole).toInt();
	myData->getMirrorByNum(index)->switchIsShow();
	updateVtk();
}

void mainWindow::on_modifyingRestriction()
{
	if (isExistenceOpenWin)
	{
		// �Ѿ��д��ڴ���
		QMessageBox::warning(NULL, "Warning",
			"A window has been opened. Please close and continue!");

		return;
	}
	int index1 = rightSelectItem->data(2, Qt::UserRole).toInt();
	int indexRestriction = rightSelectItem->data(3, Qt::UserRole).toInt();
	Restriction* tempPtr = myData->getMirrorByNum(index1)
		->getRestriction(indexRestriction - 1);
	isNew = false;
	tempRestriction = new Restriction(*tempPtr);
	on_restriction();
}

void mainWindow::on_delRestriction()
{
	int index = rightSelectItem->data(2, Qt::UserRole).toInt();
	int index2 = rightSelectItem->data(3, Qt::UserRole).toInt();
	myData->getMirrorByNum(index)->removeRestriction(index2-1);
	// �Ժ��������Restriction���±���
	int tempNum = mirrorTreeWidgetItem[index]->childCount();
	for (int i = index2 + 1; i < tempNum; ++i)
	{
		mirrorTreeWidgetItem[index]->child(i)->setData(3,
			Qt::UserRole, QVariant(i-1));
	}
	mirrorTreeWidgetItem[index]->removeChild(mirrorTreeWidgetItem[index]->child(index2));
	updateLight();
	updateVtk();
}

void mainWindow::on_modifyingMirror()
{
	MirrorTypeWidget mirrorTypeWidget;
	if (mirrorTypeWidget.exec() != QDialog::Accepted)
	{
		exit(1);
	}
}

void mainWindow::on_modifyParameters()
{
	int varInt = rightSelectItem->data(1, Qt::UserRole).toInt();
	int index = rightSelectItem->data(2, Qt::UserRole).toInt();
	renderer->RemoveActor(myData->getMirrorByNum(index)->getActor());
	switch (varInt)
	{
	case PARABOLOID:
		tempMirror = MirrorFactory::cloneMirror(myData->getMirrorByNum(index));
		tempMirror->setSelected(true);
		renderer->AddActor(tempMirror->getActor());
		on_createParaboloid();
		break;
	default:
		break;
	}
	updateVtk();
}

void mainWindow::on_restriction()
{
	if (isExistenceOpenWin)
	{
		// �Ѿ��д��ڴ���
		QMessageBox::warning(NULL, "Warning",
			"A window has been opened. Please close and continue!");

		return;
	}
	if(isNew)
		tempRestriction = new Restriction;
	renderer->AddActor(tempRestriction->getActor());
	restrictionWidget = new RestrictionWidget();
	restrictionWidget->setWindowFlags(Qt::WindowStaysOnTopHint); // �Ӵ��ڱ����ö�
	connect(restrictionWidget, SIGNAL(sendData(int)),
		this, SLOT(toReceiveRestriction(int)));

	restrictionWidget->setRestriction(tempRestriction);
	restrictionWidget->show();
	isExistenceOpenWin = true;
}

void mainWindow::toReceiveRestriction(int index)
{
	if (1 == index) // ���ȷ��
	{
		int index1; 
		
		if (!isNew)
		{
			index1 = rightSelectItem->data(2, Qt::UserRole).toInt();
			int indexRestriction = rightSelectItem->data(3, Qt::UserRole).toInt();
			myData->getMirrorByNum(index1)->setRestriction(indexRestriction - 1,
				tempRestriction);
			mirrorTreeWidgetItem[index1]->insertChild(indexRestriction,
				tempRestriction->getTree());
			mirrorTreeWidgetItem[index1]->removeChild(
				mirrorTreeWidgetItem[index1]->child(indexRestriction));

			isNew = true;
		}
		else
		{
			index1 = rightSelectItem->data(1, Qt::UserRole).toInt();
			// ����tree �в�����
			myData->getMirrorByNum(index1)->addRestriction(tempRestriction);

			int tempNum = mirrorTreeWidgetItem[index1]->childCount();
			mirrorTreeWidgetItem[index1]->addChild(tempRestriction->getTree());
			mirrorTreeWidgetItem[index1]->child(tempNum)->setData(2,
				Qt::UserRole, QVariant(index1));
			mirrorTreeWidgetItem[index1]->child(tempNum)->setData(3,
				Qt::UserRole, QVariant(tempNum));
		}

		renderer->RemoveActor(myData->getMirrorByNum(index1)->getActor());
		renderer->RemoveActor(tempRestriction->getActor());

		renderer->AddActor(myData->getMirrorByNum(index1)->getActor());

		tempRestriction = nullptr;
		delete restrictionWidget;
		restrictionWidget = nullptr;
		isExistenceOpenWin = false;
		mirrorTreeWidgetItem[index1]->setExpanded(true);
		updateLight();
	}
	else if (0 == index)// ���ȡ��
	{
		renderer->RemoveActor(tempRestriction->getActor());
	
		delete tempRestriction;
		tempRestriction = nullptr;
		delete restrictionWidget;
		restrictionWidget = nullptr;
		isExistenceOpenWin = false;
		if (!isNew)
		{
			isNew = true;
		}
	}
	updateVtk();
}

void mainWindow::createGaussian()
{
	if (isExistenceOpenWin)
	{
		// �Ѿ��д��ڴ���
		QMessageBox::warning(NULL, "Warning",
			"A window has been opened. Please close and continue!");

		return;
	}
	tempMirror = MirrorFactory::getMirror(PLANEMIRROR, GraphTrans());
	tempMirror->setSelected(true);
	renderer->AddActor(tempMirror->getActor());
	gaussianWidget = new GaussianWidget;
	gaussianWidget->setMirror(tempMirror);
	gaussianWidget->setWindowFlags(Qt::WindowStaysOnTopHint); // �Ӵ��ڱ����ö�
	gaussianWidget->show();

	connect(gaussianWidget, SIGNAL(sendData(int)),
		this, SLOT(toReceiveGaussian(int)));

	isExistenceOpenWin = true;

}

void mainWindow::toReceiveGaussian(int caseIndex)
{
	if (1 == caseIndex)
	{
		if (nullptr != myData->getSourceField()) // �������Դ�� ��Ḳ����ǰ��Դ
		{
			// �ж��Ƿ���ԭ������������
			switch (QMessageBox::question(this, tr("Question"),
				tr("Whether or not to cover the original field?"),
				QMessageBox::Ok | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				renderer->RemoveActor(myData->getSourceField()->getActor());
				renderer->RemoveActor(myData->getSourceField()->getActor3D());
				soucreFieldTreeItem->removeChild(soucreFieldTreeItem->child(0));
				break;
			case QMessageBox::No:
				toReceiveGaussian(0);
				break;
			case QMessageBox::Cancel:
				return;
			default:
				break;
			}
		}
		Field * temPtr;
		if (!gaussianWidget->getField(temPtr))
		{
			return;
		}
		renderer->AddActor(temPtr->getActor());

		soucreFieldTreeItem->addChild(temPtr->getTree());
		myData->setSourceField(temPtr);
		toReceiveGaussian(0);
	}
	else if (0 == caseIndex)// ���ȡ��
	{
		renderer->RemoveActor(tempMirror->getActor());

		delete tempMirror;
		tempMirror = nullptr;
		delete gaussianWidget;
		gaussianWidget = nullptr;
		isExistenceOpenWin = false;
	}

	updateVtk();
}

void mainWindow::on_createParaboloid()
{ 
	if (isExistenceOpenWin)
	{
		// �Ѿ��д��ڴ���
		QMessageBox::warning(NULL, "Warning",
			"A window has been opened. Please close and continue!");

		return;
	}
	paraboloidWidget = new ParaboloidWidget();
	paraboloidWidget->setWindowFlags(Qt::WindowStaysOnTopHint); // �Ӵ��ڱ����ö�

	connect(paraboloidWidget, SIGNAL(sendData(int)),
		this, SLOT(toReceiveParaboloid(int)));

	paraboloidWidget->setMirror(tempMirror);
	paraboloidWidget->show();
	isExistenceOpenWin = true;
	
}

void mainWindow::toReceiveParaboloid(int caseIndex)
{
	if (1 == caseIndex) // ���ȷ��
	{
		
		int index1 = rightSelectItem->data(2, Qt::UserRole).toInt();
		bool isFlag = false;
		// �ж��Ƿ���ԭ������������
		switch (QMessageBox::question(this, tr("Question"), 
			tr("Whether or not the existing restrictions are retained?"),
			QMessageBox::Ok | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Ok))
		{
		case QMessageBox::Ok:
			//displayTextEdit->setText(tr("ѯ�ʰ�ť / ȷ��"));
			tempMirror->moveRestriction(myData->getMirrorByNum(index1));
			isFlag = true;
			break;
		case QMessageBox::No:
			isFlag = false;
			break;
		case QMessageBox::Cancel:
			return;
		default:
			break;
		}


		tempMirror->setSelected(false);
		if (isFlag) // ����
		{
			mirrorTreeWidgetItem[index1]->removeChild(mirrorTreeWidgetItem[index1]->child(0));
		}
		else // ������
		{
			QTreeWidgetItem *childMirror = new QTreeWidgetItem;
			childMirror->setText(0, tr("Mirror") + QString::number(index1+1));
			childMirror->setData(0, Qt::UserRole, QVariant(0));
			childMirror->setData(1, Qt::UserRole, QVariant(index1));
			
			geometryTreeItem->removeChild(mirrorTreeWidgetItem[index1]);
			geometryTreeItem->insertChild(index1,childMirror);
			delete mirrorTreeWidgetItem[index1];
			mirrorTreeWidgetItem[index1] = childMirror;
		}
		
		myData->setMirror(index1, tempMirror);
		mirrorTreeWidgetItem[index1]->insertChild(0,tempMirror->getTree());
		mirrorTreeWidgetItem[index1]->child(0)->setData(2,
			Qt::UserRole, QVariant(index1));

		tempMirror = nullptr;
		delete paraboloidWidget;
		paraboloidWidget = nullptr;
		isExistenceOpenWin = false;
		updateLight();
	}
	else if (0 == caseIndex)// ���ȡ��
	{
		renderer->RemoveActor(tempMirror->getActor());
		int index1 = rightSelectItem->data(2, Qt::UserRole).toInt();

		renderer->AddActor(myData->getMirrorByNum(index1)->getActor());
		delete tempMirror;
		tempMirror = nullptr;
		delete paraboloidWidget;
		paraboloidWidget = nullptr;
		isExistenceOpenWin = false;
	}
	updateVtk();
}

void mainWindow::on_PVVA()
{
	CalculationWidget calculationDialog;
	if (calculationDialog.exec() != QDialog::Accepted)
	{
		return;
	}
	double dis = calculationDialog.getDistance();
	int numMirror = calculationDialog.getMirrorNum();
	Field * temPtr = myData->calculateByPVVA(dis, numMirror);
	renderer->AddActor(temPtr->getActor());

	QTreeWidgetItem * tree = new QTreeWidgetItem(QStringList
	(QString("Field")+QString::number(fieldNum)));
	tree->setData(0, Qt::UserRole, QVariant(FIELD));
	tree->setData(1, Qt::UserRole, QVariant(fieldNum));
	fieldTreeItem->addChild(tree);
	fieldNum++;
	updateVtk();
	//myData->
}

void mainWindow::on_treeWidget_ContextMenuRequested(QPoint pos)
{
	rightSelectItem = treeWidget->itemAt(pos);
	if (rightSelectItem == NULL)
	{
		return;
	}
	QVariant var = rightSelectItem->data(0, Qt::UserRole);

	if (var == 0)      //data(...)���ص�data�Ѿ���֮ǰ�����ڵ�ʱ��setdata()���ú�  
	{
		//�˵����ֵ�λ��Ϊ��ǰ����λ��  
		if (R_BlankMenu->isEmpty())
		{
			int index = rightSelectItem->data(1, Qt::UserRole).toInt();
			if (myData->getMirrorByNum(index)->getIsShow())
			{
				isShowMirrorAction->setChecked(true);
			}
			else
			{
				isShowMirrorAction->setChecked(false);
			}
			if (myData->getMirrorByNum(index)->getIsTransparent())
			{
				isTransparentAction->setChecked(true);
			}
			else
			{
				isTransparentAction->setChecked(false);
			}
			R_Tree_MirrorTypeMenu->exec(QCursor::pos());

		}		
	}
	else if (var == 1)     
	{
		//�˵����ֵ�λ��Ϊ��ǰ����λ��  
		if (R_BlankMenu->isEmpty())
		{
			R_Tree_MirrorParMenu->exec(QCursor::pos());

		}
	}
	else if (var == 2)
	{
		if (R_BlankMenu->isEmpty())
		{
			R_Tree_RestrictionMenu->exec(QCursor::pos());

		}
	}
}

void mainWindow::on_treeWidget_leftPressed(QTreeWidgetItem * item, int column)
{
	if (item->parent() != NULL)     // ���ڵ�
		if (qApp->mouseButtons() == Qt::LeftButton)
		{
			//RectangleWidget dialog(this, isSet); 
			if (item->data(0, Qt::UserRole) == 0)
			{
				int num = item->data(1, Qt::UserRole).toInt();
				//myData->getMirrorByNum(num)->setSelected();
			}
			if (item->data(0, Qt::UserRole) == FIELD)
			{
				int index = item->data(1, Qt::UserRole).toInt();
				leftSelectItem = item;
				showDetails(index);
			}

		}
	updateVtk();
}

void mainWindow::on_Details_FieldClicked()
{
	int index = leftSelectItem->data(1, Qt::UserRole).toInt();
	Field *tempField = myData->getFieldByNum(index);
	int dim = dimensionGroupBtn->checkedId();
	if (dim == 0) // 3D
	{
		tempField->set3D(true);
		renderer->RemoveActor(tempField->getActor());
		renderer->RemoveActor(tempField->getActor3D());
		tempField->setShowPara(fieldGroupBtn->checkedId(),
			powerGroupBtn->checkedId(), pmGroupBtn->checkedId());
		magnitudeBtn->setChecked(true);
		phaseBtn->setEnabled(false);
		magnitudeBtn->setEnabled(false);
		tempField->calActor3D();
		renderer->AddActor(tempField->getActor3D());
	}
	else // 2D
	{
		tempField->set3D(false);
		phaseBtn->setEnabled(true);
		magnitudeBtn->setEnabled(true);

		int i = pmGroupBtn->checkedId();
		if (i == 0)  // ��λû��db��ʽ
		{
			linearBtn->setEnabled(true);
			dbBtn->setEnabled(true);
		}
		else
		{
			linearBtn->setEnabled(false);
			dbBtn->setEnabled(false);
		}
		tempField->setShowPara(fieldGroupBtn->checkedId(),
			powerGroupBtn->checkedId(), i);

		renderer->RemoveActor(tempField->getActor());
		renderer->RemoveActor(tempField->getActor3D());
		tempField->calActor();
		renderer->AddActor(tempField->getActor());
	}

	updateVtk();
}

void mainWindow::updateVtk()
{
	auto window = widget.GetRenderWindow();

	//window->AddRenderer(renderer);
	window->Render();
}

void mainWindow::updateLight()
{
	myData->getDefaultLightShow()->updateData();
}

void mainWindow::showDetails(int index)
{
	int content; bool isLinear; bool isPhs;
	Field *tempField = myData->getFieldByNum(index);
	tempField->getShowPara(content, isLinear, isPhs);

	switch (tempField->get3D())
	{
	case 0:
		TwoDBtn->setChecked(true);
		phaseBtn->setEnabled(true);
		magnitudeBtn->setEnabled(true);
		break;
	case 1:
		ThreeDBtn->setChecked(true);
		magnitudeBtn->setChecked(true);
		phaseBtn->setEnabled(false);
		magnitudeBtn->setEnabled(false);
		break;
	default:
		break;
	}

	switch (content)
	{
	case 0:
		ExBtn->setChecked(true);
		break;
	case 1:
		EyBtn->setChecked(true);
		break;
	case 2:
		EzBtn->setChecked(true);
		break;
	case 3:
		HxBtn->setChecked(true);
		break;
	case 4:
		HyBtn->setChecked(true);
		break;
	case 5:
		HzBtn->setChecked(true);
		break;
	default:
		break;
	}

	switch (isPhs)
	{
	case true:
		phaseBtn->setChecked(true);
		linearBtn->setEnabled(false);
		dbBtn->setEnabled(false);
		break;
	case false:
		magnitudeBtn->setChecked(true);
		linearBtn->setEnabled(true);
		dbBtn->setEnabled(true);
		break;
	default:
		break;
	}

	switch (isLinear)
	{
	case true:
		linearBtn->setChecked(true);
		break;
	case false:
		dbBtn->setChecked(true);
		break;
	default:
		break;
	}

	if (0 == index)  // �����Դ EzHxHyHz ���õ�
	{
		EzBtn->setEnabled(false);
		HxBtn->setEnabled(false);
		HyBtn->setEnabled(false);
		HzBtn->setEnabled(false);
	}
	else
	{
		EzBtn->setEnabled(true);
		HxBtn->setEnabled(true);
		HyBtn->setEnabled(true);
		HzBtn->setEnabled(true);
	}
	
	detailsDockWidget->show();
}
