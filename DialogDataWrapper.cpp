#include "StdAfx.h"
#include "DialogDataWrapper.h"
#include "GSViewer.h"
#include <algorithm>
#include "LogStream_1.h"
#include "LpsTktStringHelpersCore.h"
#include "LpsGlobalIdPtr.h"
#include "LpsGlobals.h"
#include <assert.h>
#include <string>


using namespace Lps;
using namespace LpsTkt;
using namespace LpsTktGSM;
using namespace LpsTktGS;
using namespace LpsTktGSV;


#define _GLOBALINTIDANDBUSID ;

#pragma region DefineStatements
// -------------- Simple macros to speed up dialog data handling
#define IMPLEMENT_PUT_TEXT( ID, MEMBER ) \
	SetDlgItemText(hDlg, ID, MEMBER.c_str());

#define IMPLEMENT_GET_TEXT( ID, MEMBER ) \
{ \
	Uint32 size = GetDlgItemText(hDlg, ID, buffer, maxStringSize); \
	MEMBER = buffer; \
}

#define IMPLEMENT_PUT_CHECK( ID, MEMBER ) \
	CheckDlgButton(hDlg, ID, MEMBER ? 1 : 0);

#define IMPLEMENT_GET_CHECK( ID, MEMBER ) \
	MEMBER = (IsDlgButtonChecked(hDlg, ID) == BST_CHECKED);

#define IMPLEMENT_PUT_INTEGER( ID, MEMBER ) \
{ \
	std::ostringstream os; \
	os << std::dec << MEMBER; \
	SetDlgItemText(hDlg, ID, os.str().c_str()); \
}

#define IMPLEMENT_GET_INTEGER( ID, MEMBER ) \
{ \
	Uint32 size = GetDlgItemText(hDlg, ID, buffer, maxStringSize); \
	std::istringstream is(buffer); \
	is >> std::dec >> MEMBER; \
}

#define IMPLEMENT_PUT_REAL( ID, MEMBER ) \
{ \
	std::ostringstream os; \
	os << MEMBER; \
	SetDlgItemText(hDlg, ID, os.str().c_str()); \
}

#define IMPLEMENT_GET_REAL( ID, MEMBER ) \
{ \
	Uint32 size = GetDlgItemText(hDlg, ID, buffer, maxStringSize); \
	std::istringstream is(buffer); \
	is >> MEMBER;  \
}

#define IMPLEMENT_PUT_GUID( ID, MEMBER ) \
{ \
	CharString guidStr = toGlobalIdString(MEMBER); \
	SetDlgItemText(hDlg, ID, guidStr.c_str()); \
}

#define IMPLEMENT_GET_GUID( ID, MEMBER ) \
{ \
	Uint32 size = GetDlgItemText(hDlg, ID, buffer, maxStringSize); \
	fromGlobalIdString(buffer, MEMBER); \
}

#define IMPLEMENT_PUT_INTEGER_CHAR( ID, MEMBER ) \
{ \
	std::ostringstream os; \
	os << std::dec << (Int32)MEMBER; \
	SetDlgItemText(hDlg, ID, os.str().c_str()); \
}

#define IMPLEMENT_GET_INTEGER_CHAR( ID, MEMBER ) \
{ \
	Int32 i; \
	Uint32 size = GetDlgItemText(hDlg, ID, buffer, maxStringSize); \
	std::istringstream is(buffer); \
	is >> std::dec >> i; \
	MEMBER = i; \
}
#pragma endregion

// The one at at time pointer to a dialog wrapper instance
DialogDataWrapper * DialogDataWrapper::pDdwM = NULL;

// Constructor and Destructor
DialogDataWrapper::DialogDataWrapper(void)
{
	pDdwM = this;

	filePathM = "C:/Users/Umit/Desktop/LeapsDatabases/10ktship with even less stuff no catalog";
	fileNameM = "10KT_Ship";

	filePathM2 = "C:/Users/Umit/Desktop/LeapsDatabases";
	fileNameM2 = "Sample_System";

	pFactoryM1 = new FactoryPtr;
	*pFactoryM1 = Factory::create();

	pFactoryM2 = new FactoryPtr;
	*pFactoryM2 = Factory::create();

	hInstM = NULL;
	hWndM = NULL;
	pGSViewerM = NULL;
	lastDialogRunResultM = 0;
	pMessageStreamM = NULL;

	LpsDB1 = NULL;
	LpsActiveConcept1 = NULL;
	LpsActiveStudy1 = NULL;
	LpsDB2 = NULL;
	LpsActiveConcept2 = NULL;
	LpsActiveStudy2 = NULL;

	ElectZoneBoundary.clear();
	ez_counter = 0;

	//debug only - remove these you don't need them
	counter0 = 0;
	counter1 = 0;
	counter2 = 0;
	topologicalViewCount1 = 0;
}

DialogDataWrapper::~DialogDataWrapper(void)
{
	pDdwM = NULL;

	if (pFactoryM1 && pFactoryM1->isValid())
	{
		try
		{
			(*pFactoryM1)->closeDatabase();
			(*pFactoryM1)->destroy();
			delete pFactoryM1;
			//throw your own exception msg here 
		}
		catch (...) //catches everything including catch (Lps::Error& err)
		{
			//Lps::CharString error = err.errorMessage();
			if (pFactoryM1)
			{
				delete[] pFactoryM1;
				pFactoryM1 = NULL;
			}
		}
	}
	if (pFactoryM2 && pFactoryM2->isValid())
	{
		try
		{
			(*pFactoryM2)->closeDatabase();
			(*pFactoryM2)->destroy();
			delete pFactoryM2;

		}
		catch (...)  //catches everything including catch (Lps::Error& err)
		{
			//Lps::CharString error = err.errorMessage();
			if (pFactoryM2)
			{
				delete[] pFactoryM2;
				pFactoryM2 = NULL;
			}
		}
	}
}

Lps::CharString DialogDataWrapper::IsNameValid(Lps::CharString origName, Lps::ConceptPtr& concept)
{
	LpsTkt::StringHelpersCore strHelper;
	Lps::CharString newName;
	Lps::CharString str;

	str.append(origName);
	namecounter++;
	str = strHelper.concatenateNameAndNumber(str, namecounter);
	Lps::CharString busSection = concept->name();
	str.append(busSection);

#ifndef _GLOBALINTIDANDBUSID
#define _GLOBALINTIDANDBUSID
	//function argument:      std::vector<Lps::Uint32> gloInt  function argument
	//function call:          std::vector<Lps::Uint32> G_int = (*it)->globalId()->toIntegers();
	i = gloInt[0];
	str = strHelper.concatenateNameAndNumber(str, i);
#endif	_GLOBALINTIDANDBUSID

	return str;
}

Lps::CharString DialogDataWrapper::IsNameMapAvail(Lps::CharString origName, Lps::Uint32 option)
{
	Lps::CharString it;
	if (option == 1)
	{
		it = c1connections.find(origName)->second;
		return it;
	}
	else if (option == 2)
	{
		it = c1nodes.find(origName)->second;
		return it;
	}
	else if (option == 3)
	{
		it = c1components.find(origName)->second;
		return it;
	}
	else
		return "ERROR";
}

void DialogDataWrapper::Function0()
{
	if (LpsActiveConcept1 != NULL && LpsActiveConcept2 != NULL)
	{
		Function01(LpsActiveConcept1, LpsActiveConcept2);   //create connections
		Function02(LpsActiveConcept1, LpsActiveConcept2);   //create nodes
		Function03(LpsActiveConcept1, LpsActiveConcept2);   //create diagrams
		Function04(LpsActiveConcept1, LpsActiveConcept2);   //associate nodes with connections
		Function05(LpsActiveConcept1, LpsActiveConcept2);   //add nodes to connections
		Function06(LpsActiveConcept1, LpsActiveConcept2);   //create remaining common views in structure
		Function07(LpsActiveConcept1, LpsActiveConcept2);	//copy components and association
		Function08(LpsActiveConcept1, LpsActiveConcept2);	//create properties of the component

		LpsActiveConcept2 = NULL;
		c1connections.clear();
		c1nodes.clear();
	}
}

void DialogDataWrapper::Function01(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	ConnectionPtrList cpList = concept2->getConnections();
	Lps::CharString name1, name2;
	Lps::Uint32 version1;

	for (Lps::ConnectionPtrList::iterator it = cpList.begin(); it != cpList.end(); ++it)
	{
		(*it)->getNameAndVersion(name1, version1);

		name2 = IsNameValid(name1, LpsActiveConcept2);
		concept1->createConnection(name2, version1);
		c1connections.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name2));

		rootConnDiag = (*it)->getDiagramUsingConnection();
		if (rootConnDiag != NULL)
		{
			nameX = name2;
			versionX = version1;
		}
	}
}

void DialogDataWrapper::Function02(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	NodePtrList npList = concept2->getNodes();
	Lps::CharString name1, name2;
	Lps::Uint32 version1;

	for (Lps::NodePtrList::iterator it = npList.begin(); it != npList.end(); ++it)
	{
		(*it)->getNameAndVersion(name1, version1);

		name2 = IsNameValid(name1, LpsActiveConcept2);
		concept1->createNode(name2, version1, (*it)->location());
		c1nodes.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name2));
	}
}

void DialogDataWrapper::Function03(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::CharString name1, name2;
	Lps::Uint32 version1;
	Lps::DiagramPtrList diagrams = concept2->getDiagrams();

	for (Lps::DiagramPtrList::iterator it = diagrams.begin(); it != diagrams.end(); ++it)
	{
		(*it)->getNameAndVersion(name1, version1);

		name2 = IsNameValid(name1, LpsActiveConcept2);
		Lps::DiagramPtr newDiagram = concept1->createDiagram(name2, version1);
		c1diagrams.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name2));
		Lps::ConnectionPtr newConnection = concept1->getConnection(nameX, versionX);
		newDiagram->setRootConnection(newConnection);
	}
}

void DialogDataWrapper::Function07(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::ComponentPtrList compolist2 = getComponentsOfTheConcept(concept2);

	Lps::CharString name1, name2, name3, name4, name5;  //change to std::vector
	Lps::Uint32 version1, version2, version3, version4, version5; //change to std::vector
	const Lps::StructurePtr conceptStructure2 = concept2->getConceptStructure();
	const Lps::StructurePtr conceptStructure1 = concept1->getConceptStructure();
	int ic = 0;

	for (Lps::ComponentPtrList::iterator it1 = compolist2.begin(); it1 != compolist2.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		std::vector<Lps::Uint32> G_int = (*it1)->globalId()->toIntegers();
		name2 = IsNameValid(name1, LpsActiveConcept2);

		Lps::ComponentPtr newComponent = concept1->createComponent(name2, version1);
		c1components.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name2));

		Lps::CartesianLocation cartLoc = (*it1)->location(); // get orig coords
		newComponent->location(cartLoc); //set new coords
		c1cartesians.insert(std::pair<Lps::CharString, Lps::CartesianLocation>(name2, cartLoc));

		CommonViewPtrList cvList = (*it1)->getCommonViewsUsingComponent();
		for (Lps::CommonViewPtrList::iterator it2 = cvList.begin(); it2 != cvList.end(); ++it2)
		{
			(*it2)->getNameAndVersion(name2, version2);
			if (!conceptStructure1->doesCommonViewExist(name2, version2))
			{
				Lps::CommonViewPtr newcv = conceptStructure1->createCommonView(name2, version2);

				if (!newComponent->doesCommonViewUsingComponentExist(newcv->uniqueId()))
				{
					newComponent->addCommonViewUsingComponent(newcv);
				}
			}
			else
			{
				Lps::CommonViewPtr oldcv = conceptStructure1->getCommonView(name2, version2);

				if (!newComponent->doesCommonViewUsingComponentExist(oldcv->uniqueId()))
				{
					newComponent->addCommonViewUsingComponent(oldcv);
				}
			}
		}

		ConnectionPtrList cpList = (*it1)->getConnectionsUsingComponent();
		if (!cpList.empty())
		{
			for (Lps::ConnectionPtrList::iterator it3 = cpList.begin(); it3 != cpList.end(); ++it3)
			{
				(*it3)->getNameAndVersion(name3, version3);
				name3 = IsNameMapAvail(name3, 1);
				Lps::ConnectionPtr oldConnection = concept1->getConnection(name3, version3);

				if (!newComponent->doesConnectionUsingComponentExist(oldConnection->uniqueId()))
				{
					oldConnection->addMember(newComponent);
				}
			}
		}

		NodePtrList npList = (*it1)->getNodesUsingComponent();
		for (Lps::NodePtrList::iterator it4 = npList.begin(); it4 != npList.end(); ++it4)
		{
			(*it4)->getNameAndVersion(name4, version4);
			name4 = IsNameMapAvail(name4, 2);
			Lps::NodePtr newNode = concept1->getNode(name4, version4);
			newNode->addComponentUsingNode(newComponent);
		}
	}
}

void DialogDataWrapper::SetCartesianCoordsOfComponent(const Lps::ComponentPtr & newComp, const Lps::CartesianLocation newLoc)
{
	newComp->location(newLoc);
}

void DialogDataWrapper::Function04(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::CharString name1;
	Lps::CharString name3;

	Lps::Uint32 version1, version3;
	Lps::ConnectionPtrList conList1 = concept2->getConnections();
	counter1 = 0; counter2 = 0;
	for (Lps::ConnectionPtrList::iterator it1 = conList1.begin(); it1 != conList1.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		name1 = IsNameMapAvail(name1, 1);

		//if (concept1->doesConnectionExist(name1, version1));

		Lps::ConnectionPtrList conList3 = (*it1)->getConnectionsUsingConnection();
		for (Lps::ConnectionPtrList::iterator it3 = conList3.begin(); it3 != conList3.end(); ++it3)
		{
			(*it3)->getNameAndVersion(name3, version3);
			name3 = IsNameMapAvail(name3, 1);

			//if (concept1->doesConnectionExist(name3, version1));  

			Lps::ConnectionPtr newCon1 = concept1->getConnection(name3, version3);
			Lps::ConnectionPtr newCon2 = concept1->getConnection(name1, version1);
			newCon1->addMember(newCon2);
			c1mapctc.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name3));
			name3 = "NULL";
		}
	}
}

void DialogDataWrapper::Function08(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::ComponentPtrList components2 = LpsActiveConcept2->getComponents();
	Lps::ComponentPtrList components1 = LpsActiveConcept1->getComponents();
	Lps::CharString name1, name2, name3;
	Lps::Uint32 version1, version2, version3;

	for (Lps::ComponentPtrList::iterator it1 = components2.begin(); it1 != components2.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		std::vector<Lps::Uint32> G_int = (*it1)->globalId()->toIntegers();
		name1 = IsNameMapAvail(name1, 3);

		//Lps::Uint32 num1 = (*it1)->numberOfProperties();
		//Lps::Uint32 num2 = (*it1)->numberOfPropertyGroups();
		//Lps::PropertyGroupPtr p2 = (*it1)->getPropertyGroup(name1, version1);
		//Lps::PropertyGroupPtrList p3 = (*it1)->getPropertyGroupsFromCatalogReference();
		//Lps::PropertyGroupPtrList p4 = (*it1)->getPropertyGroups();
		//Lps::PlacementPointPtrList p5 = (*it1)->getPlacementPoints();
		//Lps::PropertyGroupPtr p6 = (*it1)->getPropertyGroupFromCatalogReference(name, version);
		//Lps::PropertyPtr p7 = getPropertyFromCatalogReference(name, version);

		Lps::PropertyPtrList p1 = (*it1)->getProperties();  //get all properties of *this component in concept 2

		if (!p1.empty())
		{
			for (Lps::PropertyPtrList::iterator it2 = p1.begin(); it2 != p1.end(); ++it2)
			{
				(*it2)->getNameAndVersion(name2, version2);
				//std::vector<Lps::Uint32> G_int = (*it2)->globalId()->toIntegers();                  
				//if ((*it1)->doesPropertyExist(name2, version2)   )

				Lps::PropertyPtr newProperty = (*it1)->getProperty(name2, version2);  //get the specific property of 
				Lps::String p_data;
				newProperty->getData(p_data);

				for (Lps::ComponentPtrList::iterator it3 = components1.begin(); it3 != components1.end(); ++it3)
				{
					(*it3)->getNameAndVersion(name3, version3);
					if (name3 == name1)
					{
						name2 = IsNameValid(name2, LpsActiveConcept2);
						(*it3)->createProperty(name2, version2, p_data);
					}
				}
			}
		}
	}
}

void DialogDataWrapper::Function06(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::CharString name1, name2, name3;
	Lps::Uint32 version1, version2, version3;
	const Lps::StructurePtr shipStructure1 = concept1->getConceptStructure();
	const Lps::StructurePtr shipStructure2 = concept2->getConceptStructure();
	Lps::CommonViewPtrList cvList2 = shipStructure2->getCommonViews();
	Lps::CommonViewPtrList cvList1 = shipStructure1->getCommonViews();

	for (Lps::CommonViewPtrList::iterator it2 = cvList2.begin(); it2 != cvList2.end(); ++it2)
	{
		(*it2)->getNameAndVersion(name2, version2);
		Lps::CommonViewPtrList cvList3 = (*it2)->getCommonViews();
		cvl.push_back(cvList3);
		if (!shipStructure1->doesCommonViewExist(name2, version2))
		{
			Lps::CommonViewPtr newCommonView = shipStructure1->createCommonView(name2, version2);
		}
	}
}

void DialogDataWrapper::Function05(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2)
{
	Lps::CharString name1, name2;
	Lps::Uint32 version1, version2;

	Lps::ConnectionPtrList conList = concept2->getConnections();

	for (Lps::ConnectionPtrList::iterator it1 = conList.begin(); it1 != conList.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		std::vector<Lps::Uint32> G_int = (*it1)->globalId()->toIntegers();
		name1 = IsNameMapAvail(name1, 1);

		Lps::NodePtrList npList = (*it1)->getNodes();

		for (Lps::NodePtrList::iterator it2 = npList.begin(); it2 != npList.end(); ++it2)
		{
			(*it2)->getNameAndVersion(name2, version2);
			std::vector<Lps::Uint32> G_int = (*it2)->globalId()->toIntegers();
			name2 = IsNameMapAvail(name2, 2);

			Lps::ConnectionPtr newConnection = concept1->getConnection(name1, version1);
			Lps::NodePtr newNode = concept1->getNode(name2, version2);
			newConnection->addMember(newNode);
			c1addntc.insert(std::pair<Lps::CharString, Lps::CharString>(name1, name2));
		}
	}
}

void DialogDataWrapper::Sorter()
{
	int length = Tvec.size();

	for (int i = 0; i < length; ++i)
	{
		bool swapped = false;
		for (int j = 0; j < length - (i + 1); ++j)
		{
			if (Tvec[j].Tx > Tvec[j + 1].Tx)
			{
				Tobj tmp = Tvec[j];
				Tvec[j] = Tvec[j + 1];
				Tvec[j + 1] = tmp;
				swapped = true;
			}
			if (Tvec[j].Tx == Tvec[j + 1].Tx)
			{
				if (Tvec[j].Ty > Tvec[j + 1].Ty)
				{
					Tobj tmp = Tvec[j];
					Tvec[j] = Tvec[j + 1];
					Tvec[j + 1] = tmp;
					swapped = true;
				}
			}
		}
		if (!swapped) break;
	}
}
/*
void DialogDataWrapper::SetCoords(const Lps::CartesianLocation & cartLoc, Lps::Uint32 num)
{
	
	Lps::Real64 x = cartLoc.x();
	Lps::Real64 y = cartLoc.y();
	Lps::Real64 z = cartLoc.z();

	switch (num)
	{            
		case 3:
		{ 
			if (ucount % 1 == 0) 
			{ 
				cartLoc.x(x), cartLoc.y(y), cartLoc.z(z);  x++;
			}  
			if (ucount % 2 == 0)
			{
				cartLoc.x(x), cartLoc.y(y), cartLoc.z(z);  x++;
			}
		}     
		case 4:
		{
			
			
		}
		default: {}
	}
	
	
}
*/
void DialogDataWrapper::BuildCompVector()
{
	Lps::CharString name1;
	Lps::Uint32 version1;
	Lps::ComponentPtrList compolist1 = LpsActiveConcept1->getComponents();
	Lps::CartesianLocation cartloc;
	Lps::Uint32 counter = 0;
	Lps::Uint32 n = 0;

	for (Lps::ComponentPtrList::iterator it1 = compolist1.begin(); it1 != compolist1.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		cartloc = (*it1)->location();
		
		std::string str(name1);
		std::size_t found1 = str.find("electricalCable");
		std::size_t found2 = str.find("electricalBusSwitchgear");
		std::size_t found3 = str.find("ship_0000004");
		std::size_t found4 = str.find("ship_0000005");
		std::size_t found5 = str.find("ship_0000006");

		if (found1 != std::string::npos && found4 != std::string::npos)
		{   
			//SetCoords(cartloc, n = 3);
			M_C.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Mid", name1, (*it1) ));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Mid", name1, (*it1)));
		}
		else if (found2 != std::string::npos && found4 != std::string::npos)
		{
			//SetCoords(cartloc, n = 4);
			M_SG.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Mid", name1, (*it1)));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Mid", name1, (*it1)));
		}
		else if (found1 != std::string::npos && found5 != std::string::npos)
		{
			//SetCoords(cartloc, n = 5);
			A_C.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Aft", name1, (*it1)));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Aft", name1, (*it1)));
		}
		else if (found3 != std::string::npos && found2 != std::string::npos)
		{
			//SetCoords(cartloc, n = 1);
			F_SG.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Fwd", name1, (*it1)));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Fwd", name1, (*it1)));
		}
		else if (found2 != std::string::npos && found5 != std::string::npos)
		{
			//SetCoords(cartloc, n = 6);
			A_SG.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Aft", name1, (*it1)));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "SwitchGear", "Aft", name1, (*it1)));
		}
		else if (found1 != std::string::npos && found3 != std::string::npos)
		{
			//SetCoords(cartloc, n = 2);
			F_C.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Fwd", name1, (*it1)));
			Tvec.push_back(Tobj(cartloc.x(), cartloc.y(), cartloc.z(), counter, "Cable", "Fwd", name1, (*it1)));
		}
		else
		{
			name1 = "";
		}
		counter++;
	}
	Sorter();  // Now sort component objects based on thier location in the bus but without using std::sort - do it the hard way!
}

Lps::ComponentPtr DialogDataWrapper::NextComponent()
{
	Lps::ComponentPtr target_comp;

	//sort them according to location only and re-sort them inside compovec

	return target_comp;
}

Lps::CharString DialogDataWrapper::linkTwoComponents()
{
	// add ports and terminals to components, 
	// then creates appropriate connections:  portTerminalLinkage, exchangeConnection, componentExchangeConnection,
	// then either creates or adds to appropriate systemConnection and Diagram
	Lps::CharString textMessage;
	Lps::ComponentPtrList focusComponents = pDdwM->LpsActiveConcept1->getComponents();

	if (focusComponents.size() == 0)
	{
		textMessage = "This concept has no components";
		return textMessage;
	}

	//	get two components
	//	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_LINKTWOCOMPONENTS, hWndM, (DLGPROC)linkTwoComponentsDialog);
	//	Function10();

	activeComponentM2 = NextComponent();
	activeComponentM1 = NextComponent();
	
	if (lastDialogRunResultM != IDOK)
	{
		textMessage = "Canceled";
		return textMessage;
	}
	if (activeComponentM1 == activeComponentM2)
	{
		textMessage = "FAIL: the same component was selected twice";
		return textMessage;
	}

	// create port and terminal on first component

	// create and name port
	Lps::CharString portName = IsNameValid("port", LpsActiveConcept1);
	Lps::NodePtr port1 = LpsActiveConcept1->createNode(portName, 1, activeComponentM1->location());
	port1->descriptiveName(activeComponentM1->descriptiveName() + " - " + port1->descriptiveName());
	port1->addComponentUsingNode(activeComponentM1);

	// create and name terminal
	Lps::CharString terminalName = IsNameValid("terminal", LpsActiveConcept1);
	Lps::NodePtr terminal1 = LpsActiveConcept1->createNode(terminalName, 1, activeComponentM1->location());
	terminal1->descriptiveName(activeComponentM1->descriptiveName() + " - " + terminal1->descriptiveName());

	// connect port and terminal
	Lps::CharString connectionName = IsNameValid("portTerminalLinkage", LpsActiveConcept1);
	Lps::ConnectionPtr connection = LpsActiveConcept1->createConnection(connectionName, 1);
	connection->addMember(port1);
	connection->addMember(terminal1);

	//create port and terminal on second component

	// create and name port
	portName = IsNameValid("port", LpsActiveConcept1);
	Lps::NodePtr port2 = LpsActiveConcept1->createNode(portName, 1, activeComponentM2->location());
	port2->descriptiveName(activeComponentM2->descriptiveName() + " - " + port2->descriptiveName());
	port2->addComponentUsingNode(activeComponentM2);

	// create and name terminal
	terminalName = IsNameValid("terminal", LpsActiveConcept1);
	Lps::NodePtr terminal2 = LpsActiveConcept1->createNode(terminalName, 1, activeComponentM2->location());
	terminal2->descriptiveName(activeComponentM2->descriptiveName() + " - " + terminal2->descriptiveName());

	// connect port and terminal
	connectionName = IsNameValid("portTerminalLinkage", LpsActiveConcept1);
	connection = LpsActiveConcept1->createConnection(connectionName, 1);
	connection->addMember(port2);
	connection->addMember(terminal2);

	// create exchange connection
	connectionName = IsNameValid("exchangeConnection", LpsActiveConcept1);
	Lps::ConnectionPtr exchangeConnection = LpsActiveConcept1->createConnection(connectionName, 1);
	exchangeConnection->addMember(terminal1);
	exchangeConnection->addMember(terminal2);

	//create component exchange connection
	connectionName = IsNameValid("componentExchangeConnection", LpsActiveConcept1);
	Lps::ConnectionPtr componentExchangeConnection = LpsActiveConcept1->createConnection(connectionName, 1);
	componentExchangeConnection->addMember(exchangeConnection);
	componentExchangeConnection->addMember(activeComponentM1);
	componentExchangeConnection->addMember(activeComponentM2);

	//create system structure if needed
	DialogDataWrapper::createSystems();

	//select system
	DialogDataWrapper::selectSystem();

	//find or create diagram for system
	Lps::DiagramPtrList diagramList = LpsActiveSystemM0->getDiagrams();
	Lps::DiagramPtr diagram;

	if (diagramList.size() == 1)
	{
		diagram = diagramList[0];
	}
	else if (diagramList.size() == 0)
	{
		Lps::CharString diagramName = IsNameValid("systemConnectivity", LpsActiveConcept1);
		diagram = LpsActiveConcept1->createDiagram(diagramName, 1);
		diagram->descriptiveName("Electrical System Diagram");
		LpsActiveSystemM0->addDiagram(diagram);
	}
	else
	{
		textMessage = "FAIL:  more than one diagram in system";
		return textMessage;
	}

	//find or create root connection for diagram
	Lps::ConnectionPtr systemConnection;
	if (diagram->doesRootConnectionExist())
	{
		systemConnection = diagram->getRootConnection();
	}
	else
	{
		Lps::CharString connectionName = IsNameValid("systemConnection", LpsActiveConcept1);
		systemConnection = LpsActiveConcept1->createConnection(connectionName, 1);
		systemConnection->descriptiveName("Electrical System Connection");
		diagram->setRootConnection(systemConnection);
	}

	//add this component exchange connection to the system connection (root connection)
	systemConnection->addMember(componentExchangeConnection);

	// zero out active components and return
	activeComponentM1 = NULL;
	activeComponentM2 = NULL;
	LpsActiveSystemM0 = NULL;
	textMessage = "SUCCESS:  components have been linked";
	return textMessage;
}

void DialogDataWrapper::AddCompExContoRootSysCon()
{
	Lps::CharString name1;
	Lps::Uint32 version1;
	Lps::ConnectionPtrList conList = LpsActiveConcept1->getConnections();
	Lps::ConnectionPtr rootSysCon;

	for (Lps::ConnectionPtrList::iterator it1 = conList.begin(); it1 != conList.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
		std::string str(name1);
		std::size_t found1 = str.find("componentExchangeConnection");
		std::size_t found2 = str.find("ship_0000002");
		std::size_t found3 = str.find("systemConnection");

		if (found1 != std::string::npos && found2 == std::string::npos)
			rootSysConVector.push_back(name1);
		if (found3 != std::string::npos && found2 != std::string::npos)
			rootSysCon = LpsActiveConcept1->getConnection(name1, 1);
	}
	for (Lps::CharString name2 : rootSysConVector)
	{
		if (LpsActiveConcept1->doesConnectionExist(name2, version1))
		{
			const Lps::ConnectionPtr & CEC = LpsActiveConcept1->getConnection(name2, version1);
			rootSysCon->addMember(CEC);
		}
	}
}

void DialogDataWrapper::createSystems()
{
	LpsTkt::StringHelpersCore strHelper;
	Lps::SystemPtr shipSystemsM, electricSystemsM;

	if (!LpsActiveConcept1->doesSystemExist("shipSystems", 1))
		shipSystemsM = LpsActiveConcept1->createSystem("shipSystems", 1);
	else
		shipSystemsM = LpsActiveConcept1->getSystem("shipSystems", 1);

	if (LpsActiveConcept1->doesSystemExist("electricSystems", 1))
		electricSystemsM = LpsActiveConcept1->getSystem("electricSystems", 1);
	else
		electricSystemsM = LpsActiveConcept1->createSystem("electricSystems", 1);
	if (!shipSystemsM->doesSystemExist("electricSystems", 1))
		shipSystemsM->addSystem(electricSystemsM);
}

LRESULT CALLBACK DialogDataWrapper::linkTwoComponentsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	HWND cmb_box1, cmb_box2;

	switch (message) {
	case WM_INITDIALOG:
	{
		cmb_box1 = GetDlgItem(hDlg, IDC_COMPONENTNAMECOMBO);
		cmb_box2 = GetDlgItem(hDlg, IDC_COMPONENTNAMECOMBO2);
		Lps::ComponentPtrList focusComponents = pDdwM->LpsActiveConcept1->getComponents();
		for (Lps::ComponentPtr comp : focusComponents)
		{
			SendMessage(cmb_box1, CB_ADDSTRING, 0, (LPARAM)_T(comp->descriptiveName().c_str()));
			SendMessage(cmb_box2, CB_ADDSTRING, 0, (LPARAM)_T(comp->descriptiveName().c_str()));
		}
		SendMessage(cmb_box1, CB_SETCURSEL, 0, 0);
		SendMessage(cmb_box2, CB_SETCURSEL, 0, 0);
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK)
			{
				Lps::ComponentPtrList focusComponents = pDdwM->LpsActiveConcept1->getComponents();
				cmb_box1 = GetDlgItem(hDlg, IDC_COMPONENTNAMECOMBO);
				LRESULT selection = SendMessage(cmb_box1, CB_GETCURSEL, 0, 0);
				pDdwM->activeComponentM1 = focusComponents[selection];
				cmb_box2 = GetDlgItem(hDlg, IDC_COMPONENTNAMECOMBO2);
				selection = SendMessage(cmb_box2, CB_GETCURSEL, 0, 0);
				pDdwM->activeComponentM2 = focusComponents[selection];
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void DialogDataWrapper::selectSystem()
{
	Lps::SystemPtrList systemList = pDdwM->LpsActiveConcept1->getSystems();

	Lps::CharString name1;
	Lps::Uint32 version1;

	for (Lps::SystemPtrList::iterator it1 = systemList.begin(); it1 != systemList.end(); ++it1)
	{
		(*it1)->getNameAndVersion(name1, version1);
	}
	pDdwM->LpsActiveSystemM0 = systemList[0];
}

void DialogDataWrapper::NumOfFacesInView(const Lps::CommonViewPtr& cvp)
{
	LpsTkt::Inspector inspector;
	Lps::FacePtrList fx;
	numoffaces.push_back(inspector.findFaceList(cvp, fx));
}   //don't need it for calculations 

Lps::Real64List DialogDataWrapper::GetBoundaryOfElectricZones(int *dbNum)  //you can also pass electzone as param const Lps::CommonViewPtr& cvp
{
	if (*dbNum == 1)
	{
		const Lps::StructurePtr shipStructure1 = LpsActiveConcept1->getConceptStructure();   //make this line generic to get rid of dbNum completely
		Lps::FacePtrList faces = shipStructure1->getFaces();
		Lps::Real64List x, y, z;
		Lps::CharString name0, name1, name2;
		Lps::Uint32 version0, version1, version2;

		Lps::CommonViewPtrList eZoneViews = getElectViews(shipStructure1);
		//if (cvp == eZoneViews[id])  //if pointer cvp is in eZoneView (it has to be) you have a match for electricZone_000000X

		for (int id1 = 0; id1 < eZoneViews.size(); id1++)
		{
			Lps::CommonViewPtr cvp = eZoneViews[id1];  //you just hit the first "electricZone_000000X" in the list. do something! 
			Lps::Uint32 topologicalViewCount = cvp->numberOfTopologicalViews();
			Lps::TopologicalViewPtrList const & topviews = cvp->getTopologicalViews();
			cvp->getNameAndVersion(name0, version0);
			counter0++;
			for (int id2 = 0; id2 < topologicalViewCount; id2++)
			{
				Lps::TopologicalViewPtr tvp = topviews[id2];
				(tvp)->getNameAndVersion(name1, version1);
				counter1++;
				for (Lps::FacePtrList::iterator it = faces.begin(); it != faces.end(); it++)
				{
					const Lps::TopologicalViewPtr twp = (*it)->getTopologicalView();
					const Lps::UniqueId uID = (*it)->getUidOfTopologicalView();
					(*it)->getNameAndVersion(name2, version2);
					if (tvp == twp)  //that's it. we have a hit. add it to the basket
					{
						//const Lps::Real64List & bbox = tvp->getFace()->cartesianBounds();
						const Lps::Real64List & bbox = (*it)->cartesianBounds();
						x.push_back(bbox[0]);
						x.push_back(bbox[3]);
						y.push_back(bbox[1]);
						y.push_back(bbox[4]);
						z.push_back(bbox[2]);
						z.push_back(bbox[5]);
						counter2++;

						std::sort(x.begin(), x.end());
						std::sort(y.begin(), y.end());
						std::sort(z.begin(), z.end());
					}  // end of if
				} //end of face loop
			} //end of topological view loop

			if (x.empty() || y.empty() || z.empty())
			{
				return ElectZoneBoundary;
			}

			//ElectZoneBoundary.push_back(counter0);
			ElectZoneBoundary.push_back(x.front());
			ElectZoneBoundary.push_back(y.front());
			ElectZoneBoundary.push_back(z.front());
			ElectZoneBoundary.push_back(x.back());
			ElectZoneBoundary.push_back(y.back());
			ElectZoneBoundary.push_back(z.back());

			x.clear();
			y.clear();
			z.clear();

			counter1 = 0;
			counter2 = 0;

		} // end of common view loop

		for (int idx = 0; idx < ElectZoneBoundary.size(); idx = idx + 3)
			xsort.push_back(ElectZoneBoundary[idx]);
		for (int idy = 1; idy < ElectZoneBoundary.size(); idy = idy + 3)
			ysort.push_back(ElectZoneBoundary[idy]);
		for (int idz = 2; idz < ElectZoneBoundary.size(); idz = idz + 3)
			zsort.push_back(ElectZoneBoundary[idz]);

		std::sort(xsort.begin(), xsort.end());
		std::sort(ysort.begin(), ysort.end());
		std::sort(zsort.begin(), zsort.end());
	}
	return ElectZoneBoundary;
}

Lps::CommonViewPtrList DialogDataWrapper::getElectViews(const Lps::StructurePtr& pp)
{
	Lps::CommonViewPtrList const & cvlist1 = pp->getCommonViews();
	Lps::CommonViewPtrList eviews;

	for (Lps::CommonViewPtrList::const_iterator cvp1 = cvlist1.begin(); cvp1 != cvlist1.end(); ++cvp1)
	{
		if ((*cvp1)->name().find("electricZone_") != -1)
		{
			eviews.push_back(*cvp1);
			mymap1.insert(std::pair<Lps::CharString, Lps::CharString>((*cvp1)->name(), (*cvp1)->getGlobalIdAsString()));
			ezoneNames.push_back((*cvp1)->name());
			ez_counter++;
		}
	}
	return eviews;
}

Lps::ComponentPtrList DialogDataWrapper::getComponentsOfTheConcept(const Lps::ConceptPtr& cp)
{
	Lps::ComponentPtrList const & clist1 = cp->getComponents();
	Lps::ComponentPtrList complist3;

	for (Lps::ComponentPtrList::const_iterator cvp1 = clist1.begin(); cvp1 != clist1.end(); ++cvp1)
	{
		complist3.push_back(*cvp1);
		//mymap3.insert(std::pair<Lps::CharString, Lps::CharString>((*cvp1)->name(), (*cvp1)->getGlobalIdAsString()));
	}
	return complist3;
}

Lps::ConnectionPtrList DialogDataWrapper::getConnectionsOfTheConcept(const Lps::ConceptPtr& cp)
{
	Lps::ConnectionPtrList const & clist = cp->getConnections();
	Lps::ConnectionPtrList conlist;

	for (Lps::ConnectionPtrList::const_iterator conp = clist.begin(); conp != clist.end(); ++conp)
	{
		conlist.push_back(*conp);
		//mymap4.insert(std::pair<Lps::CharString, Lps::CharString>((*conp)->name(), (*conp)->getGlobalIdAsString()));
	}
	return conlist;
}

Lps::Uint32 DialogDataWrapper::GetNumOfElectZones(int *dbNum)
{
	if (*dbNum == 1)
	{
		ez_counter = 0;
		mymap1.clear();
		if (!pFactoryM1 || !pFactoryM1->isValid() || LpsDB1 == NULL || LpsActiveConcept1 == NULL)
		{
			ez_counter = -1;
			return ez_counter;
		}
		Lps::StructurePtr shipStructure1 = LpsActiveConcept1->getConceptStructure();
		getElectViews(shipStructure1);
		return ez_counter;
	}
}

// Opening Databases, creating factories
bool DialogDataWrapper::getAndOpenLeapsDatabase1() {
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_FILE_OPEN, hWndM, (DLGPROC)fileDialog1);
	if (lastDialogRunResultM != IDOK)
		return false;
	return openDatabase1(filePathM, fileNameM);
}

bool DialogDataWrapper::selectStudy1()
{
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_SELECTSTUDY, hWndM, (DLGPROC)selectStudyDialog1);
	if (lastDialogRunResultM != IDOK)
		return false;
	return true;
}

LRESULT CALLBACK DialogDataWrapper::fileDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Load the dialog's controls
		BOOL OK = TRUE;
		IMPLEMENT_PUT_TEXT(IDC_FILEPATH, pDdwM->filePathM)
			IMPLEMENT_PUT_TEXT(IDC_DBNAME, pDdwM->fileNameM)
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {

				// Get the dialogs control data
				UINT size;
				IMPLEMENT_GET_TEXT(IDC_FILEPATH, pDdwM->filePathM)
					IMPLEMENT_GET_TEXT(IDC_DBNAME, pDdwM->fileNameM)
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

std::vector<std::string> DialogDataWrapper::getShipConcepts1()
{
	std::vector<std::string> conceptList;
	Lps::StudyPtrList studies = LpsDB1->getStudies();

	for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
	{
		std::string str = (*it)->descriptiveName();
		std::string gID = (*it)->getGlobalIdAsString();
		if (str.find(activeStudyName1) != -1)
		{
			Lps::ConceptPtrList concepts = LpsDB1->getStudyWithGuid(gID)->getConcepts();
			for (int i = 0; i < concepts.size(); ++i)
			{
				conceptList.push_back(concepts[i]->descriptiveName());
				conceptList.push_back(concepts[i]->getGlobalIdAsString());
			}
		}
	}

	return conceptList;
}

std::vector<std::string> DialogDataWrapper::getShipStudies1()
{
	std::vector<std::string> studyList;
	Lps::StudyPtrList studies = LpsDB1->getStudies();

	for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
	{
		{
			for (int i = 0; i < studies.size(); ++i)
			{
				studyList.push_back(studies[i]->descriptiveName());
				studyList.push_back(studies[i]->getGlobalIdAsString());
			}
		}
	}
	return studyList;
}

LRESULT CALLBACK DialogDataWrapper::selectStudyDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	HWND cmb_box1;

	switch (message)
	{
	case WM_INITDIALOG:      //populates list of studies in the dialog box
	{
		cmb_box1 = GetDlgItem(hDlg, IDC_STUDYNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
		std::vector<std::string> studyNames = pDdwM->getShipStudies1();  // list of study descriptive names and GUIDs
		for (size_t i = 0; i < studyNames.size(); i += 2)
		{
			SendMessage(cmb_box1, CB_ADDSTRING, 0, (LPARAM)_T(studyNames[i].c_str()));
		}
		SendMessage(cmb_box1, CB_SETCURSEL, 0, 0);
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {
				cmb_box1 = GetDlgItem(hDlg, IDC_STUDYNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
				LRESULT selection = SendMessage(cmb_box1, CB_GETCURSEL, 0, 0);  //which selection number was picked?
				char selectedStudyName[100];
				GetDlgItemText(cmb_box1, selection, selectedStudyName, 100);
				//				pDdwM->activeStudyName = std::string(selectedStudyName);
				std::vector<std::string> studyNames = pDdwM->getShipStudies1();  //list of study names and GUIDs
				pDdwM->activeStudyName1 = studyNames[selection * 2];
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK DialogDataWrapper::selectConceptDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	HWND cmb_box1;

	switch (message) {
	case WM_INITDIALOG:      //populates list of concepts in the dialog box
	{
		cmb_box1 = GetDlgItem(hDlg, IDC_CONCEPTNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
		std::vector<std::string> conceptNames = pDdwM->getShipConcepts1();  //list of concept names and GUIDs
		for (size_t i = 0; i < conceptNames.size(); i += 2)
		{
			SendMessage(cmb_box1, CB_ADDSTRING, 0, (LPARAM)_T(conceptNames[i].c_str()));
		}
		SendMessage(cmb_box1, CB_SETCURSEL, 0, 0);
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {
				cmb_box1 = GetDlgItem(hDlg, IDC_CONCEPTNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
				LRESULT selection = SendMessage(cmb_box1, CB_GETCURSEL, 0, 0);  //which selection number was picked?
				char selectedConceptName[100];
				GetDlgItemText(cmb_box1, selection, selectedConceptName, 100);
				//				pDdwM->activeConceptName = std::string(selectedConceptName);
				std::vector<std::string> conceptNames = pDdwM->getShipConcepts1();  //list of concept names and GUIDs
				pDdwM->activeConceptName1 = conceptNames[selection * 2];
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool DialogDataWrapper::selectConcept1()
{
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_SELECTCONCEPT, hWndM, (DLGPROC)selectConceptDialog1);
	if (lastDialogRunResultM != IDOK)
		return false;
	std::vector<std::string> conceptList = getShipConcepts1();
	std::string conceptGuid;

	for (size_t i = 0; i < conceptList.size(); i = i + 2)
	{
		if (conceptList[i].find(activeConceptName1) != -1)
		{
			conceptGuid = conceptList[i + 1];
		}
	}
	if (!conceptGuid.empty())
	{
		return selectConcept1(conceptGuid.c_str());
	}
	return false;
}

bool DialogDataWrapper::openDatabase1(const CharString& filePath, const CharString& dbName)
{
	try
	{
		if (!pFactoryM1->isValid() || !(*pFactoryM1)->doesDatabaseExist(dbName, filePath))
		{
			return false;
		}
		LpsDB1 = (*pFactoryM1)->openDatabase(dbName, filePath);

		//if (LpsDB1->isLocked())
		//LpsDB1->unlock();
	}
	catch (Error& err)
	{
		std::cerr << err.what();
		return false;
	}
	return true;
}

void DialogDataWrapper::closeDatabase1()
{
	// this can take awhile
	if (pFactoryM1 && pFactoryM1->isValid())
	{
		try
		{
			//const Lps::GlobalId * guid = LpsActiveStudy1->globalId();
			//(*pFactoryM1)->destroyStudy(guid);

			(*pFactoryM1)->closeDatabase();
			(*pFactoryM1)->destroy();  //also destroy the factory pointer along with it
		}
		catch (Lps::Error&err)
		{
			std::cerr << err.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Failed to close Database possibly due to corruption!" << std::endl;
		}
	}
	LpsDB1 = NULL;
	LpsActiveConcept1 = NULL;
	LpsActiveStudy1 = NULL;
}

bool DialogDataWrapper::selectConcept1(const char* gID)
{
	if (LpsActiveConcept1 != NULL)
	{
		//Reset all to NULL	
	}
	try
	{
		Lps::StudyPtrList studies = LpsDB1->getStudies();
		for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
		{
			std::string str = (*it)->descriptiveName();
			if (str.find(activeStudyName1) != -1)
			{
				LpsActiveConcept1 = (*it)->getConceptWithGuid(gID);
			}
			else
			{
				return false;
			}
		}
	}
	catch (Lps::Error& err)
	{
		Lps::CharString error = err.errorMessage();
		return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//----------------------------------------------------------Database 2 Duplicate Class Methods-----------------------------------------------------------

// Opening Databases, creating factories
bool DialogDataWrapper::getAndOpenLeapsDatabase2() {
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_FILE_OPEN, hWndM, (DLGPROC)fileDialog2);
	if (lastDialogRunResultM != IDOK)
		return false;
	return openDatabase2(filePathM2, fileNameM2);    //pass filenames of the second db
}

bool DialogDataWrapper::selectStudy2()
{
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_SELECTSTUDY, hWndM, (DLGPROC)selectStudyDialog2);
	if (lastDialogRunResultM != IDOK)
		return false;
	return true;
}

LRESULT CALLBACK DialogDataWrapper::fileDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Load the dialog's controls
		BOOL OK = TRUE;
		IMPLEMENT_PUT_TEXT(IDC_FILEPATH, pDdwM->filePathM2)
			IMPLEMENT_PUT_TEXT(IDC_DBNAME, pDdwM->fileNameM2)
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{

			if (LOWORD(wParam) == IDOK) {

				// Get the dialogs control data
				UINT size;
				IMPLEMENT_GET_TEXT(IDC_FILEPATH, pDdwM->filePathM2)
					IMPLEMENT_GET_TEXT(IDC_DBNAME, pDdwM->fileNameM2)
			}

			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

std::vector<std::string> DialogDataWrapper::getShipConcepts2()
{
	std::vector<std::string> conceptList;
	Lps::StudyPtrList studies = LpsDB2->getStudies();

	for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
	{
		std::string str = (*it)->descriptiveName();
		std::string gID = (*it)->getGlobalIdAsString();
		if (str.find(activeStudyName2) != -1)
		{
			Lps::ConceptPtrList concepts = LpsDB2->getStudyWithGuid(gID)->getConcepts();
			for (int i = 0; i < concepts.size(); ++i)
			{
				conceptList.push_back(concepts[i]->descriptiveName());
				conceptList.push_back(concepts[i]->getGlobalIdAsString());
			}
		}
	}
	return conceptList;
}

std::vector<std::string> DialogDataWrapper::getShipStudies2()
{
	std::vector<std::string> studyList;
	Lps::StudyPtrList studies = LpsDB2->getStudies();

	for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
	{
		//	std::string str = (*it)->uniqueId();
		//	std::string gID = (*it)->getGlobalIdAsString();
		{
			for (int i = 0; i < studies.size(); ++i)
			{
				studyList.push_back(studies[i]->descriptiveName());
				studyList.push_back(studies[i]->getGlobalIdAsString());
			}
		}
	}
	return studyList;
}

LRESULT CALLBACK DialogDataWrapper::selectStudyDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	HWND cmb_box1;

	switch (message)
	{
	case WM_INITDIALOG:      //populates list of studies in the dialog box
	{
		cmb_box1 = GetDlgItem(hDlg, IDC_STUDYNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
		std::vector<std::string> studyNames = pDdwM->getShipStudies2();  // list of study descriptive names and GUIDs
		for (size_t i = 0; i < studyNames.size(); i += 2)
		{
			SendMessage(cmb_box1, CB_ADDSTRING, 0, (LPARAM)_T(studyNames[i].c_str()));
		}
		SendMessage(cmb_box1, CB_SETCURSEL, 0, 0);
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {
				cmb_box1 = GetDlgItem(hDlg, IDC_STUDYNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
				LRESULT selection = SendMessage(cmb_box1, CB_GETCURSEL, 0, 0);  //which selection number was picked?
				char selectedStudyName[100];
				GetDlgItemText(cmb_box1, selection, selectedStudyName, 100);
				//				pDdwM->activeStudyName = std::string(selectedStudyName);
				std::vector<std::string> studyNames = pDdwM->getShipStudies2();  //list of study names and GUIDs
				pDdwM->activeStudyName2 = studyNames[selection * 2];
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK DialogDataWrapper::selectConceptDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int maxStringSize = 2048;
	char buffer[maxStringSize + 1];

	HWND cmb_box1;

	switch (message) {
	case WM_INITDIALOG:      //populates list of concepts in the dialog box
	{
		cmb_box1 = GetDlgItem(hDlg, IDC_CONCEPTNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
		std::vector<std::string> conceptNames = pDdwM->getShipConcepts2();  //list of concept names and GUIDs
		for (size_t i = 0; i < conceptNames.size(); i += 2)
		{
			SendMessage(cmb_box1, CB_ADDSTRING, 0, (LPARAM)_T(conceptNames[i].c_str()));
		}
		SendMessage(cmb_box1, CB_SETCURSEL, 0, 0);
	}
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {
				cmb_box1 = GetDlgItem(hDlg, IDC_CONCEPTNAMECOMBO);  //Retrieves a handle to a control in the specified dialog box
				LRESULT selection = SendMessage(cmb_box1, CB_GETCURSEL, 0, 0);  //which selection number was picked?
				char selectedConceptName[100];
				GetDlgItemText(cmb_box1, selection, selectedConceptName, 100);
				//				pDdwM->activeConceptName = std::string(selectedConceptName);
				std::vector<std::string> conceptNames = pDdwM->getShipConcepts2();  //list of concept names and GUIDs
				pDdwM->activeConceptName2 = conceptNames[selection * 2];
			}
			pDdwM->lastDialogRunResultM = LOWORD(wParam);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool DialogDataWrapper::selectConcept2()
{
	DialogBox(hInstM, (LPCTSTR)IDD_DIALOG_SELECTCONCEPT, hWndM, (DLGPROC)selectConceptDialog2);
	if (lastDialogRunResultM != IDOK)
		return false;
	std::vector<std::string> conceptList = getShipConcepts2();
	std::string conceptGuid;

	for (size_t i = 0; i < conceptList.size(); i = i + 2)
	{
		if (conceptList[i].find(activeConceptName2) != -1)
		{
			conceptGuid = conceptList[i + 1];
		}
	}
	if (!conceptGuid.empty())
	{
		return selectConcept2(conceptGuid.c_str());
	}
	return false;
}

bool DialogDataWrapper::openDatabase2(const CharString& filePath, const CharString& dbName)
{
	try
	{
		if (!pFactoryM2->isValid() || !(*pFactoryM2)->doesDatabaseExist(dbName, filePath))
		{
			return false;
		}
		LpsDB2 = (*pFactoryM2)->openDatabase(dbName, filePath);

		//if (LpsDB1->isLocked())
		///LpsDB1->unlock();
	}
	catch (Error& err)
	{
		std::cerr << err.what();
		return false;
	}
	return true;
}

void DialogDataWrapper::closeDatabase2()
{
	// this can take awhile
	if (pFactoryM2 && pFactoryM2->isValid())
	{
		try
		{
			(*pFactoryM2)->closeDatabase();
			(*pFactoryM2)->destroy();  //also destroy the factory pointer along with it
		}
		catch (Lps::Error&err)
		{
			std::cerr << err.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Failed to close Database possibly due to corruption!" << std::endl;
		}
	}
	LpsDB2 = NULL;
	LpsActiveConcept2 = NULL;
	LpsActiveStudy2 = NULL;
}

bool DialogDataWrapper::selectConcept2(const char* gID)
{
	if (LpsActiveConcept2 != NULL)
	{
		//Reset all to NULL	
	}
	try
	{
		Lps::StudyPtrList studies = LpsDB2->getStudies();
		for (Lps::StudyPtrList::iterator it = studies.begin(); it != studies.end(); ++it)
		{
			std::string str = (*it)->descriptiveName();
			if (str.find(activeStudyName2) != -1)
			{
				LpsActiveConcept2 = (*it)->getConceptWithGuid(gID);
			}
			else
			{
				return false;
			}
		}
	}
	catch (Lps::Error& err)
	{
		Lps::CharString error = err.errorMessage();
		return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

 
