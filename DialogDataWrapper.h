#include "LpsTktMsgStream.h"
#include "LpsTktGSView.h"
#include <math.h> 
#include "LpsTktInspector.h"
#include <map>

// Class to wrap the leaps instance and talk to dialogs using
// basic windows services, no MFC, no ATL, no .NET.

class Tobj
{
public:
	const Lps::ComponentPtr & Tcomptr;
	Lps::Real64 Tx;
	Lps::Real64 Ty;
	Lps::Real64 Tz;
	Lps::Uint32 Tuid;
	Lps::CharString Tcomptype;
	Lps::CharString Tbustype;
	Lps::CharString Tname;

	Tobj(Lps::Real64 x, Lps::Real64 y, Lps::Real64 z, Lps::Uint32 uid, Lps::CharString comptype, Lps::CharString bustype, Lps::CharString name, const Lps::ComponentPtr & comptr) : 
		Tx(x), Ty(y), Tz(z), Tuid(uid), Tcomptype(comptype), Tbustype(bustype), Tname(name), Tcomptr(comptr) {}
	~Tobj(void){}; 

	Tobj& operator=(Tobj other)
	{
		std::swap(Tx, other.Tx);
		std::swap(Ty, other.Ty);
		std::swap(Tz, other.Tz);
		std::swap(Tuid, other.Tuid);
		std::swap(Tcomptype, other.Tcomptype);
		std::swap(Tbustype, other.Tbustype);
		std::swap(Tname, other.Tname);
		return *this;
	}
private:;
};

class DialogDataWrapper
{
public:
	static DialogDataWrapper *								pDdwM;
	HINSTANCE												hInstM;
	HWND													hWndM;
	LPCSTR													caption;
	int														lastDialogRunResultM;
	LpsTktGSV::GSView*										pGSViewerM;
	Lps::Uint32												ez_counter;
	Lps::Real64List											ElectZoneBoundary;
	LpsTkt::MsgStream*										pMessageStreamM;
	std::map<Lps::CharString, Lps::CharString>              mymap1, mymap2, mymap3, mymap4;
	Lps::Real64List                                         xsort, ysort, zsort;
	std::vector<Lps::CharString>                            ezoneNames;
	std::vector<Tobj>                                       Tvec, F_C, M_C, M_SG, A_C, F_SG, A_SG;

	DialogDataWrapper(void);
	~DialogDataWrapper(void);

	std::vector<std::string> getShipStudies1();
	std::vector<std::string> getShipConcepts1();
	std::vector<std::string> getShipStudies2();
	std::vector<std::string> getShipConcepts2();

	bool selectConcept1(const char* gID);
	bool selectConcept2(const char* gID);
	bool getAndOpenLeapsDatabase1();
	bool getAndOpenLeapsDatabase2();
	bool selectConcept1();
	bool selectConcept2();
	bool selectStudy1();
	bool selectStudy2();
	bool openDatabase1(const Lps::CharString& filePath, const Lps::CharString& dbName);
	bool openDatabase2(const Lps::CharString& filePath, const Lps::CharString& dbName);
	void closeDatabase1();
	void closeDatabase2();

	void NumOfFacesInView(const Lps::CommonViewPtr& cvp);
	void Sorter();
	void VerifyCoords(const Lps::CartesianLocation & cartLoc, Lps::Uint32 num);
	void createSystems();
	void selectSystem();
	void BuildCompVector();
	void SetCartesianCoordsOfComponent(const Lps::ComponentPtr & xComp, const Lps::CartesianLocation newLoc);
	void AddCompExContoRootSysCon();
	Lps::Real64List GetBoundaryOfElectricZones(int *dbNum);
	Lps::Uint32 GetNumOfElectZones(int *dbNum);
	Lps::CommonViewPtrList getElectViews(const Lps::StructurePtr& pp);
	Lps::ComponentPtrList getComponentsOfTheConcept(const Lps::ConceptPtr& cp);
	Lps::ConnectionPtrList getConnectionsOfTheConcept(const Lps::ConceptPtr& cp);
	Lps::CharString IsNameMapAvail(Lps::CharString origName, Lps::Uint32 option);
	Lps::CharString IsNameValid(Lps::CharString origName, std::vector<Lps::Uint32> gloInt, Lps::ConceptPtr& concept);
	Lps::CharString IsNameValid(Lps::CharString origName, Lps::ConceptPtr& concept);
	Lps::ComponentPtr NextComponent();
	Lps::CharString linkTwoComponents();

	void Function0();
	void Function01(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function02(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function03(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function04(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function05(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function06(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function07(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);
	void Function08(const Lps::ConceptPtr & concept1, const Lps::ConceptPtr & concept2);

	static LRESULT CALLBACK selectConceptDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK selectStudyDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK fileDialog1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK selectConceptDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK selectStudyDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK fileDialog2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK linkTwoComponentsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Lps::DatabasePtr LpsDB1;
	Lps::DatabasePtr LpsDB2;
	Lps::ConceptPtr LpsActiveConcept1;
	Lps::ConceptPtr LpsActiveConcept2;
	Lps::StudyPtr LpsActiveStudy1;
	Lps::StudyPtr LpsActiveStudy2;
	Lps::Uint32 topologicalViewCount1, topologicalViewCount2;
	Lps::CommonViewPtr cvp;
	Lps::ComponentPtrList comp_list;
	Lps::NodePtrList node_list;
	Lps::Study* pStudy;
	Lps::DbManager* pManagerM1;
	Lps::CharString	filePathM;
	Lps::CharString	fileNameM;
	Lps::CharString	filePathM2;
	Lps::CharString	fileNameM2;
	Lps::CharString activeStudyName1;
	Lps::CharString activeConceptName1;
	Lps::CharString activeStudyName2;
	Lps::CharString activeConceptName2;
	Lps::FactoryPtr* pFactoryM1;
	Lps::FactoryPtr* pFactoryM2;
	std::vector<Lps::Uint32> numoffaces;
	Lps::CharString nameX;
	Lps::Uint32 versionX;
	Lps::DiagramPtr rootConnDiag;
	std::multimap<Lps::CharString, Lps::CharString> c1nodes, c1connections, c1diagrams, c1systems, c1components, c1mapctc, c1addntc;
	std::multimap<Lps::CharString, Lps::CartesianLocation> c1cartesians;
	Lps::ComponentPtr activeComponentM1, activeComponentM2;
	Lps::NodePtr activePortM;
	Lps::SystemPtr  LpsActiveSystemM0;
	std::vector<Lps::CharString> rootSysConVector;
	std::vector<Lps::CommonViewPtrList> cvl;
	Lps::Uint32 counter0, counter1, counter2;  //for debugging remove when done
};
