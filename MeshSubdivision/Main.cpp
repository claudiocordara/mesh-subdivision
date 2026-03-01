#pragma once

//#include "AlgoRunner.h"

//#include <Inventor/Win/SoWin.h>
//#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include "Mesh.h"
#include "Loop.h"
#include "Butterfly.h"
//#include "CatmullClark.h"
#include "KobbeltRoot3.h"
#include "PetersReiff.h"

int main(int, char** argv)
{
	Mesh* msh = new Mesh;
	msh->loadSTL("C:\\AEPSystem\\models\\cordoblasta\\default\\14 cordo nat.stl", false, false, false);
	msh->saveSTL("D:\\Sviluppo\\mesh-subdivision\\meshLoaded.stl", "", true);

	BaseScheme* scheme = new Butterfly();
	Mesh* schemeMesh = scheme->run(msh);
	delete msh;
	Mesh* schemeMesh1 = scheme->run(schemeMesh);

	schemeMesh->saveSTL("D:\\Sviluppo\\mesh-subdivision\\meshRefined1.stl", "", true);
	schemeMesh1->saveSTL("D:\\Sviluppo\\mesh-subdivision\\meshRefined2.stl", "", true);

	return 0;
	/*
	HWND window = SoWin::init(argv[0]);
	SoWinExaminerViewer* viewer = new SoWinExaminerViewer(window);
	viewer->setBackgroundColor(SbColor(0.2f, 0.2f, 0.2f));

	viewer->setDrawStyle(SoWinViewer::DrawType::INTERACTIVE,
		SoWinViewer::DrawStyle::VIEW_WIREFRAME_OVERLAY);

	SoSeparator* root = new SoSeparator();
	root->ref();

	root->addChild((new AlgoRunner())->run());

	viewer->setSceneGraph(root);
	viewer->show();

	SoWin::show(window);
	SoWin::mainLoop();

	delete viewer;
	root->unref();

	return 0;
	*/
}