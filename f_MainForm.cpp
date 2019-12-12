// ---------------------------------------------------------------------------

#include <vcl.h>
#include <shlobj.h>
#include <Shlwapi.h>
#pragma hdrstop

#include "f_MainForm.h"
#include "f_ftpConn.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "IdBaseComponent"
#pragma link "IdCoder"
#pragma link "IdCoder3to4"
#pragma link "IdCoderMIME"
#pragma resource "*.dfm"
#pragma comment(lib, "shlwapi.lib")
#pragma warn -8004
TForm1 *Form1;
int SortByColumn = 0;  //전에 선택한 칼럼 index
int SortOrder = 1;     //클릭시 오름차순 내림차순 변경
bool moveFileSelect = false; //이동할 파일 List 저장 스위치
int moveSelected = 0; //이동 시 선택한 파일 개수
int tarSelected  = 0; //이동 시 마우스가 가리키는 영역 표시
AnsiString SelectedName = "";
TWndMethod OrigProc1;
// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)
{
	listColumn1 = "파일명";
	listColumn2 = "크기";
	listColumn3 = "파일유형";
	listColumn4 = "최종 수정";
	hWnd = Application->Handle;
	s_PastDownPath = "";
}
// ---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)
{
	OleInitialize(NULL);
	DragAcceptFiles(Handle,true); //drag&drop use
	ChangeWindowMessageFilterEx(Handle, WM_DROPFILES, MSGFLT_ALLOW, NULL); //drag&drop use when admin mode
	ChangeWindowMessageFilterEx(Handle, 0x0049, MSGFLT_ALLOW, NULL);
	AnsiString Path = ExtractFilePath(Application->ExeName)+"\\user.ini";   //Read ftp info from user.ini file
	char bfer[30];
	memset(bfer, 0x00, sizeof(bfer));
	GetPrivateProfileString(TEXT("CONFIGURATION"),TEXT("IP"), "", bfer, sizeof(bfer),Path.c_str());
	Edit1->Text = IdDecode->DecodeString(bfer);
	memset(bfer, 0x00, sizeof(bfer));
	GetPrivateProfileString(TEXT("CONFIGURATION"),TEXT("ID"), "", bfer, sizeof(bfer),Path.c_str());
	Edit2->Text = IdDecode->DecodeString(bfer);
	memset(bfer, 0x00, sizeof(bfer));
	GetPrivateProfileString(TEXT("CONFIGURATION"),TEXT("PORT"), "", bfer, sizeof(bfer),Path.c_str());
	Edit4->Text = IdDecode->DecodeString(bfer);
	GetPrivateProfileString(TEXT("CONFIGURATION"),TEXT("PASSIVE"),"0",bfer,sizeof(bfer),Path.c_str());
	if(bfer[0] == '0') CheckBox1->Checked = false;
	else CheckBox1->Checked = true;
	GetPrivateProfileString(TEXT("CONFIGURATION"),TEXT("ENCODING"),"EUCKR",bfer,sizeof(bfer),Path.c_str());
	if(!strcmp(bfer, "EUCKR")) EUCKR1->Checked = true;
	else if(!strcmp(bfer, "UTF8")) UTF81->Checked = true;
	Image1->Picture->Bitmap->Handle = NULL;
	Image1->Picture = NULL;
	Edit3->SetFocus();
}
// ---------------------------------------------------------------------------
void __fastcall TForm1::FormResize(TObject *Sender)
{
	try
	{
		if(FtpConn->HFtp_Connection ==  NULL) return;
		func_ftpListViewAdd(PathBox->Text);
	}
	catch(...){}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
	FtpConn->func_ftpInit();
	HANDLE hSrch;
	WIN32_FIND_DATA localFileData;
	AnsiString newPath = ExtractFilePath(Application->ExeName)+"\\tmpDir\\";
	hSrch = FindFirstFile((newPath+"*.*").c_str(), &localFileData);
	do{
		if(localFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //directory
		{
		}
		else{  //file
			DeleteFile(newPath+localFileData.cFileName);
		}
	}while(FindNextFile(hSrch, &localFileData));
	FindClose(hSrch);
	hSrch = NULL;
	Image1->Picture->Bitmap->Handle = NULL;
	Image1->Picture = NULL;
	OleUninitialize();
	Action = caFree;
}
// ---------------------------------------------------------------------------
//ftp connection state(conn or non conn)
bool __fastcall TForm1::func_ftpConnState()
{
	if(FtpConn->HFtp_Connection == NULL || FtpConn->HFtp_Session == NULL){
		Memo1->Lines->Add("서버가 연결되어있지 않음.");
		FtpConn->func_ftpInit();
		TreeView2->Items->Clear();
		PathBox->Clear();
		Button1->Caption = "연결";
		TreeView2->Enabled = false;
		PathBox->Enabled = false;
		ListView2->Clear();
		ListView2->Enabled = false;
		Edit1->Enabled = true;
		Edit2->Enabled = true;
		Edit3->Enabled = true;
		Edit4->Enabled = true;
		return true;
	}
		return false;
}
//탐색기->프로그램 Drag&Drop, 파일 업로드
void __fastcall TForm1::WMDropFiles(TWMDropFiles &message)
{
	char fullPath[MAX_PATH];
	UINT fileCount = DragQueryFile((HDROP)message.Drop,0xffffffff,NULL,0); //Count draged file
    ListView3->Clear();
    for(UINT i=0 ;i<fileCount ;i++)
    {
		DragQueryFile((HDROP)message.Drop,i,fullPath,MAX_PATH);
		if(DirectoryExists(fullPath))                          //if draged file is directory
		{
			ListView3->Items->Add();
			ListView3->Items->Item[i]->Caption = fullPath;
			ListView3->Items->Item[i]->SubItems->Add("폴  더");
		}
		else if(FileExists(fullPath))                          // if draged file is file
		{
            ListView3->Items->Add();
			ListView3->Items->Item[i]->Caption = fullPath;
			ListView3->Items->Item[i]->SubItems->Add("파  일");
		}
		else continue;
	}
	DragFinish((HDROP)message.Drop);

	func_upLoadFile();
}
//------------------------------------------------------------------------------
//파일 업로드
void __fastcall TForm1::func_upLoadFile()
{
	if(func_ftpConnState()) return;
	int cc = 0;
	for(int i=0;i<ListView2->Items->Count;i++)    //find overlapped name
	{
		for(int j=0;j<ListView3->Items->Count;j++)
		{
			if(ListView2->Items->Item[i]->Caption == ExtractFileName(ListView3->Items->Item[j]->Caption))
			{
				ListView3->Items->Item[j]->Delete();
				cc++;
			}
		}
	}
	if(cc>0)Memo1->Lines->Add("이름이 중복된 파일(폴더)이 있습니다.("+IntToStr(cc)+")");
	if(ListView3->Items->Count == 0) return;
	int count = ListView3->Items->Count;
	Memo1->Lines->Add("업로드 준비 중...");
	Application->ProcessMessages();
	ListView2->Enabled = false;
	TreeView2->Enabled = false;
	downloadButton->Enabled = false;
	deleteButton->Enabled = false;
	createDirButton->Enabled = false;
	renameButton->Enabled = false;
	FtpConn->downCountFull = 0;
	FtpConn->ProgressBar2->Position = 0;
	for(int i=0;i<ListView3->Items->Count;i++)     //count uploaded file
	{
		if(ListView3->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			FtpConn->downCountFull++;
		}
		else if(ListView3->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			FtpConn->func_localFileCountF(ListView3->Items->Item[i]->Caption);
		}
	}
	FtpConn->ProgressBar2->Max = FtpConn->downCountFull;
	FtpConn->downCount = 0;
	int upC = 0;
	vector<String>lFile;	//local file path
	vector<String>fFile;    //ftp file path
	Memo1->Lines->Add("업로드 시작");
	Application->ProcessMessages();
	FtpConn->Show();
	for (int i = 0; i < count; i++)
	{
		if(FtpConn->downCancel == 1) break;
		bool ul = false;
		AnsiString localPath = ListView3->Items->Item[i]->Caption;
		AnsiString ftpPath   = PathBox->Text + "/" + ExtractFileName(ListView3->Items->Item[i]->Caption);
		if (ListView3->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			ul = FtpConn->func_fileUpLoad(localPath, ftpPath);
			if (ul)
				upC++;
			FtpConn->downCount++;
			StatusBar1->SimpleText = "업로드 중...("+IntToStr(upC)+")";
			Application->ProcessMessages();
		}
		else if (ListView3->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			bool fDir = FtpConn->func_localFindAllFiles(localPath, ftpPath, &lFile, &fFile);
			if (!fDir)
			{
				func_ftpListViewAdd(PathBox->Text);
				lFile.clear();
				fFile.clear();
				Memo1->Lines->Add("업로드 실패 : 폴더 내 파일을 찾을 수 없음("+localPath+")");
				continue;
			}
			for (unsigned int i = 0; i < lFile.size(); i++)
			{
				if(FtpConn->downCancel == 1) break;
				ul = FtpConn->func_fileUpLoad(lFile.at(i), fFile.at(i));
				if (ul)
					upC++;
				FtpConn->downCount++;
				StatusBar1->SimpleText = "업로드 중...("+IntToStr(upC)+")";
				Application->ProcessMessages();
			}
			lFile.clear();
			fFile.clear();
		}
	}
	FtpConn->Close();
	FtpConn->downCancel = 0;
	ListView2->Enabled = true;
	TreeView2->Enabled = true;
	createDirButton->Enabled = true;
	func_ftpListViewAdd(PathBox->Text);
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
	lFile.clear();
	fFile.clear();
	if (upC == 0)
		Memo1->Lines->Add("업로드 실패");
	else
		Memo1->Lines->Add("업로드 완료(" + IntToStr(upC) + ")");
	StatusBar1->SimpleText = "";
}

// ---------------------------------------------------------------------------
// ------------------------------아이콘 추가-------------------------------------
// ------------------------------------------------------------------------------
int __fastcall TForm1::func_ftpIconAdd(AnsiString Ext)
{
	unsigned int j = 0;
	for(j=0;j< ExtImage.size(); j++)
	{
		if(Ext == ExtImage.at(j)) return j;
    }
	TIcon *ico = new TIcon;
	SHFILEINFO sfi;
	SecureZeroMemory(&sfi, sizeof(sfi));
	::SHGetFileInfo(Ext.c_str(), 0, &sfi, sizeof(sfi),
		SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	ico->Handle = sfi.hIcon;
	iconBig->AddIcon(ico);
	::SHGetFileInfo(Ext.c_str(), 0, &sfi, sizeof(sfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	ico->Handle = sfi.hIcon;
	iconSmall->AddIcon(ico);
	delete ico;
	DestroyIcon((HICON)sfi.hIcon);
	ExtImage.push_back(Ext);
	return j;
}
// -----------------------------------------------------------------------------
int __fastcall TForm1::func_ftpDirIconAdd()
{
	TIcon *ico = new TIcon;
	SHFILEINFO sfi;
	SecureZeroMemory(&sfi, sizeof(sfi));
	::SHGetFileInfo("Doesn't matter", FILE_ATTRIBUTE_DIRECTORY, &sfi,
		sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	ico->Handle = sfi.hIcon;
	iconBig->AddIcon(ico);
	::SHGetFileInfo("Doesn't matter", FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
					SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	ico->Handle = sfi.hIcon;
	iconSmall->AddIcon(ico);
	delete ico;
	DestroyIcon((HICON)sfi.hIcon);
	ExtImage.push_back("");
	return 0;
}
// -----------------------------------------------------------------------------
// --------------------------------FTP 리스트 뷰--------------------------------
// -----------------------------------------------------------------------------
// ftp 연결
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	if (Button1->Caption == "연결")
	{
		mode = 0;
		if (CheckBox1->Checked)
		{
			mode = 0x08000000;
			Memo1->Lines->Add(Edit1->Text + ":" + Edit4->Text +
				" 서버에 접속 중(PASSIVE MODE)");
		}
		else
		{
			mode = 0;
			Memo1->Lines->Add(Edit1->Text + ":" + Edit4->Text +
				" 서버에 접속 중(ACTIVE MODE)");
		}
		FtpConn->func_ftpInit();
		bool ftpc = FtpConn->func_ftpConnect(Edit1->Text, Edit4->Text,  //ftp connect
			Edit2->Text, Edit3->Text, mode);
		if (!ftpc)
		{
			Memo1->Lines->Add("접속 실패");
			FtpConn->func_ftpInit();
			Memo1->Lines->Add("연결 해제");
			return;
		}
		Memo1->Lines->Add("/디렉토리 불러오는 중");
		func_ftpListViewAdd("/");                                //ftp listview
		TTreeNode *ftpRoot = TreeView2->Items->Add(NULL, "/");   //ftp treeview
		bool ftpt = func_ftpTreeView("/", ftpRoot);
		if (!ftpt)
		{
			Memo1->Lines->Add("디렉토리 목록 불러오기 실패");
			FtpConn->func_ftpInit();
			Memo1->Lines->Add("연결 해제");
			return;
		}
		TreeView2->Enabled = true;
		TreeView2->Selected = ftpRoot;
		PathBox->Text = "/";
		ListView2->Enabled = true;
		Edit1->Enabled = false;
		Edit2->Enabled = false;
		Edit3->Enabled = false;
		Edit4->Enabled = false;
		downloadButton->Enabled = false;
		deleteButton->Enabled = false;
		createDirButton->Enabled = true;
		renameButton->Enabled = false;
		Button1->Caption = "연결해제";
		Memo1->Lines->Add("접속 성공");
		EUCKR1->Enabled = false;
		UTF81->Enabled = false;
		AnsiString Path = ExtractFilePath(Application->ExeName)+"\\user.ini";
		AnsiString Incod = IdEncode->EncodeString(Edit1->Text);
		WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("IP"), Incod.c_str(),Path.c_str());
		Incod = IdEncode->EncodeString(Edit2->Text);
		WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("ID"), Incod.c_str(),Path.c_str());
		Incod = IdEncode->EncodeString(Edit4->Text);
		WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("PORT"), Incod.c_str(),Path.c_str());
		if(CheckBox1->Checked) WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("PASSIVE"), "1",Path.c_str());
		else WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("PASSIVE"), "0",Path.c_str());
		if(EUCKR1->Checked) WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("ENCODING"),"EUCKR",Path.c_str());
		else if(UTF81->Checked) WritePrivateProfileString(TEXT("CONFIGURATION"),TEXT("ENCODING"),"UTF8",Path.c_str());
		int s_allCount = ListView2->Items->Count-1;
		if(PathBox->Text == "/") s_allCount = s_allCount+1;
		StatusBar1->SimpleText = "전체 파일 : "+IntToStr(s_allCount) +"개";
	}
	else if (Button1->Caption == "연결해제")
	{
		FtpConn->func_ftpInit();
		TreeView2->Items->Clear();
		PathBox->Clear();
		Button1->Caption = "연결";
		TreeView2->Enabled = false;
		PathBox->Enabled = false;
		ListView2->Clear();
		ListView2->Enabled = false;
		Edit1->Enabled = true;
		Edit2->Enabled = true;
		Edit3->Enabled = true;
		Edit4->Enabled = true;
		downloadButton->Enabled = false;
		deleteButton->Enabled = false;
		createDirButton->Enabled = false;
		renameButton->Enabled = false;
		Memo1->Lines->Add("연결 해제");
		EUCKR1->Enabled = true;
		UTF81->Enabled = true;
		StatusBar1->SimpleText = "";
	}
}
// ---------------------------------------------------------------------------
// ftp 리스트 뷰 출력
bool __fastcall TForm1::func_ftpListViewAdd(AnsiString Path)
{
	if(func_ftpConnState()) return false;
	FtpConn->func_initStruct();
	iconBig->Clear();
	iconSmall->Clear();
	ExtImage.clear();
	ExtImage.reserve(0);
	ImageCount = 0;
	func_ftpDirIconAdd();
	bool output = FtpConn->func_GetFTPFileList(Path.c_str(), &FtpConn->ftpfileValue);     //file list
	if(!output)
	{
		if(GetLastError() == (unsigned long)12003)
		{
			Application->MessageBoxA(L"폴더 접근에 실패하였습니다. 권한 문제일 수 있습니다.",L"오류",0 | MB_ICONWARNING);
			FtpConn->ftpfileValue.fileName.push_back("상위폴더로 이동");
			FtpConn->ftpfileValue.fileType.push_back("");
			FtpConn->ftpfileValue.fileSize.push_back("");
			FtpConn->ftpfileValue.fileTime.push_back("");
			FtpConn->ftpfileValue.fileIcon.push_back(0);
		}
		else if(GetLastError() == (unsigned long)12031){
			FtpConn->func_ftpInit();
			FtpConn->func_ftpConnect(Edit1->Text, Edit4->Text, Edit2->Text, Edit3->Text, mode);
			func_ftpListViewAdd(PathBox->Text);
			return false;
		}
	}

	/*for(unsigned int i=0;i<FtpConn->ftpfileValue.fileName.size();i++)
	{
		if(FtpConn->ftpfileValue.fileType.at(i) == "파  일")
		{
			func_ftpIconAdd(ExtractFileExt(FtpConn->ftpfileValue.fileName.at(i)));
		}
	} */
	sort(SortOrder, SortByColumn);
	return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ListView2Data(TObject *Sender, TListItem *Item)
{
	if(Item->Index < (int)FtpConn->ftpfileValue.fileName.size())
	{
		Item->Caption = FtpConn->ftpfileValue.fileName.at(Item->Index);
		ULONGLONG sizeR = StrToInt64Def(FtpConn->ftpfileValue.fileSize.at(Item->Index), -1);
		AnsiString size = "";
		if(sizeR != -1) size = FormatFloat("#0,",sizeR);
		Item->SubItems->Add(size);
		Item->SubItems->Add(FtpConn->ftpfileValue.fileType.at(Item->Index));
		Item->ImageIndex = FtpConn->ftpfileValue.fileIcon.at(Item->Index);
		Item->SubItems->Add(FtpConn->ftpfileValue.fileTime.at(Item->Index));
	}
}
//---------------------------------------------------------------------------
// ftp 디렉토리 변경
void __fastcall TForm1::ListView2DblClick(TObject *Sender)
{
	if(func_ftpConnState()) return;
	if (!ListView2->Selected)
		return;
	if (ListView2->Selected->SubItems->Strings[1] == "파  일")
	{
		AnsiString s_FileName = ListView2->Selected->Caption;
		AnsiString s_FileSize = ListView2->Selected->SubItems->Strings[0];
		s_FileSize = StringReplace(s_FileSize, ",", "",TReplaceFlags() << rfReplaceAll);
		ULONGLONG s_FileSizeU = StrToInt64Def(s_FileSize, 0);
		if(!s_FileSizeU) return;
		AnsiString s_extFile = LowerCase(ExtractFileExt(s_FileName));
		if(s_extFile!=".jpg" && s_extFile!=".png" && s_extFile!=".tif" &&
		   s_extFile!=".bmp" && s_extFile!=".gif") return;
		if(preViewPanel->Visible == false){
			AnsiString s_FileName = ListView2->Selected->Caption;
			AnsiString s_FileSize = ListView2->Selected->SubItems->Strings[0];
			s_FileSize = StringReplace(s_FileSize, ",", "",TReplaceFlags() << rfReplaceAll);
			ULONGLONG s_FileSizeU = StrToInt64Def(s_FileSize, 0);
			if(!s_FileSizeU) return;
			FtpConn->Show();
			FtpConn->func_fileReview(s_FileName, s_FileName, s_FileSizeU);
			FtpConn->Close();
			AnsiString ExeFiles = ExtractFilePath(Application->ExeName)+"tmpDir\\"+s_FileName;
			SHELLEXECUTEINFO m_seInfo;
			ZeroMemory(&m_seInfo, sizeof(SHELLEXECUTEINFO));
			m_seInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			m_seInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			m_seInfo.lpFile = ExeFiles.c_str();
			m_seInfo.nShow = SW_SHOW;
			::ShellExecuteEx(&m_seInfo);
		}
		else
		{
			Timer1->Enabled = true;
		}
		return;
	}
	else{
		TTreeNode *treeSel;
		AnsiString Path = ListView2->Selected->Caption;
		AnsiString memoPath = Path;
		if(memoPath =="상위폴더로 이동")
		{
			memoPath = "상위";
		}
		Memo1->Lines->Add(memoPath+" 폴더로 이동 중...");
		if (Path == "상위폴더로 이동")
		{
			treeSel = TreeView2->Selected->Parent;
			TreeView2->Selected = treeSel;
			Path = "..";
		}
		else
		{
			treeSel = TreeView2->Selected->getFirstChild();
			while (treeSel)
			{
				if (treeSel->Text == Path)
					break;
				treeSel = treeSel->GetNext();
			}
		}
		treeSel->DeleteChildren();
		func_ftpTreeView(Path, treeSel);
		TreeView2->Selected = treeSel;
		TreeView2->Enabled = false;
		Application->ProcessMessages();
		PathBox->Text = ftpGetAncestor(TreeView2->Selected);
		func_ftpListViewAdd(PathBox->Text);
		TreeView2->Enabled = true;
		Memo1->Lines->Add("이동 완료.");
        int s_allCount = ListView2->Items->Count-1;
		if(PathBox->Text == "/") s_allCount = s_allCount+1;
		StatusBar1->SimpleText = "전체 파일 : "+IntToStr(s_allCount) +"개";
	}
}
// ------------------------------------------------------------------------------
// --------------------------------FTP 트리 뷰---------------------------------
// ------------------------------------------------------------------------------
bool __fastcall TForm1::func_ftpTreeView(AnsiString Dir, TTreeNode *root)
{
	unsigned long temp_Length = INTERNET_MAX_PATH_LENGTH;
	char temp_path[INTERNET_MAX_PATH_LENGTH];
	WIN32_FIND_DATA ftpFileData;
	FtpSetCurrentDirectory(FtpConn->HFtp_Connection, Dir.c_str());
	if (::FtpGetCurrentDirectory(FtpConn->HFtp_Connection, temp_path, &temp_Length)) // 현재 FTP 디렉토리 가져오기(temp_path)
	{
		HINTERNET H_Find_File = ::FtpFindFirstFile(FtpConn->HFtp_Connection,
			"*.*", &ftpFileData, 0, 0); // H_Find_File에 파일 속성 대입
		if (H_Find_File != NULL)
		{
			do
			{
				if (ftpFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if(EUCKR1->Checked) TreeView2->Items->AddChild(root, ftpFileData.cFileName);
					else if(UTF81->Checked) TreeView2->Items->AddChild(root, FtpConn->UTF8ToANSI(ftpFileData.cFileName));
				}
			}
			while (::InternetFindNextFile(H_Find_File, &ftpFileData));
		}
		::InternetCloseHandle(H_Find_File);
	}
	else
		return false;

	return true;
}
// ---------------------------------------------------------------------------
// 트리뷰 폴더 선택, 확장
void __fastcall TForm1::TreeView2MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
	if(func_ftpConnState()) return;
	if (!TreeView2->Selected) return;
	if(!TreeView2->GetNodeAt(X,Y)) return;
	if(TreeView2->Selected->Text != TreeView2->GetNodeAt(X, Y)->Text) return;
	AnsiString Path = ftpGetAncestor(TreeView2->Selected);
	Memo1->Lines->Add(Path+" 폴더로 이동 중...");
	TreeView2->Enabled = false;
	Application->ProcessMessages();
	if (TreeView2->Selected->Expanded == false)
	{
		TreeView2->Selected->DeleteChildren();
		func_ftpTreeView(Path, TreeView2->Selected);
		func_ftpListViewAdd(Path);
	}
	else
		func_ftpListViewAdd(Path);

	PathBox->Text = ftpGetAncestor(TreeView2->Selected);
	TreeView2->Enabled = true;
	Memo1->Lines->Add("이동 완료.");
}
//---------------------------------------------------------------------------
// 트리뷰 선택한 폴더 경로 가져오기
AnsiString __fastcall TForm1::ftpGetAncestor(TTreeNode *node)
{
	AnsiString Path;
	Path = node->Text;
	while (node->Parent)
	{
		node = node->Parent;
		AnsiString nodeText = node->Text;
		if(nodeText == "/") nodeText = "";
		Path = nodeText + "/" + Path;
	}
	return Path;
}
// ------------------------------------------------------------------------------
// ---------------------------팝업창 기능----------------------------------------
// ---------------------------------------------------------------------------
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
//initiate directory to download file
{
	switch (uMsg) {
		// 폴더선택 다이얼로그의 초기화가 끝난 경우
	case BFFM_INITIALIZED:
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
	}
    return 0;
}
// ---------------------------------------------------------------------------
//Select directory when download file
AnsiString __fastcall TForm1::func_dirFind()
{
	ITEMIDLIST* pidlBrowse;
	char tmp[MAX_PATH];
	memset(tmp, 0x00, sizeof(tmp));

	::BROWSEINFOA BRinfo;
	BRinfo.hwndOwner = hWnd;   // 부모 윈도우의 핸들
	BRinfo.pidlRoot = NULL;      // 브라우징 할 루트 디렉토리
	BRinfo.pszDisplayName = tmp;    // 선택할 디렉토리가 저장될 버퍼
	BRinfo.lpszTitle = "폴더를 선택하세요"; // 출력 문자열
	BRinfo.ulFlags = BIF_RETURNONLYFSDIRS|BIF_USENEWUI;  // 디렉토리 선택 옵션
	BRinfo.lpfn = BrowseCallbackProc;       // 이벤트에 대한 사용자 정의 처리 함수
	BRinfo.lParam = (LPARAM)s_PastDownPath.c_str();       // 처리 함수에 넘겨질 인수

	pidlBrowse = SHBrowseForFolder(&BRinfo); // 다이얼로그를 띄운다.
	if(pidlBrowse != NULL) {
	   SHGetPathFromIDList(pidlBrowse, tmp); // 패스를 얻어온다.
	   s_PastDownPath = tmp;
	   s_PastDownPath = s_PastDownPath+"\\";
	   return s_PastDownPath+"\\";
	}
	return "";
}
// ------------------------------------------------------------------------------
//popup, file(directory) download
void __fastcall TForm1::lv2downloadClick(TObject *Sender)
{
	AnsiString downPath = func_dirFind();
	if(downPath == "") return;
	func_downLoad(downPath);
}
//------------------------------------------------------------------------------
//file(directory) download function
void __fastcall TForm1::func_downLoad(AnsiString downPath)
{
	Memo1->Lines->Add("다운로드 준비 중...");
	Application->ProcessMessages();
	ListView2->Enabled = false;
	TreeView2->Enabled = false;
	downloadButton->Enabled = false;
	deleteButton->Enabled = false;
	createDirButton->Enabled = false;
	renameButton->Enabled = false;
	int count = 0;
	int downC = 0;
	int nDownC = 0;
	vector<String>lFile;
	vector<String>fFile;
	vector<__int64>sFile;
	ListView3->Clear();
	for(int i=0;i<ListView2->Items->Count;i++)    //선택된 리스트 개수
	{
		if(FtpConn->downCancel == 1) break;
		if(ListView2->Items->Item[i]->Selected)
		{
			ListView3->Items->Add();
			ListView3->Items->Item[count]->Caption = ListView2->Items->Item[i]->Caption;
			ListView3->Items->Item[count]->SubItems->Add(ListView2->Items->Item[i]->SubItems->Strings[1]);
			ListView3->Items->Item[count]->SubItems->Add(ListView2->Items->Item[i]->SubItems->Strings[0]);
			ListView3->Items->Item[count]->SubItems->Strings[1] = StringReplace
				(ListView3->Items->Item[count]->SubItems->Strings[1], ",", "",
				TReplaceFlags() << rfReplaceAll);
			count++;
		}
	}
	FtpConn->downCountFull = 0;
	FtpConn->ProgressBar2->Position = 0;
	for(int i=0;i<ListView3->Items->Count;i++)
	{
		if(ListView3->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			FtpConn->downCountFull++;
		}
		else if(ListView3->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			FtpConn->func_ftpFileCountF(PathBox->Text+"/"+ListView3->Items->Item[i]->Caption);
		}
	}
	FtpConn->ProgressBar2->Max = FtpConn->downCountFull;
	FtpConn->downCount = 0;
	Memo1->Lines->Add("다운로드 시작");
	Application->ProcessMessages();
	FtpConn->Show();
	for (int i = 0; i < count; i++)
	{

		bool dl = false;
		if (ListView3->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			if(!FileExists(downPath + ListView3->Items->Item[i]->Caption))
			{
				__int64 size = StrToInt64(ListView3->Items->Item[i]->SubItems->Strings[1]);
				dl = FtpConn->func_fileDownLoad(downPath + ListView3->Items->Item[i]->Caption,
										PathBox->Text + "/" + ListView3->Items->Item[i]->Caption,size);
				if (dl) downC++;
				else nDownC++;
				FtpConn->downCount++;
				StatusBar1->SimpleText = "다운로드 중...("+IntToStr(downC)+")";
				Application->ProcessMessages();
			}
			else nDownC++;
		}
		else if (ListView3->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			bool fDir = FtpConn->func_ftpFindFiles(downPath + ListView3->Items->Item[i]->Caption,
								PathBox->Text + "/" + ListView3->Items->Item[i]->Caption,&lFile, &fFile, &sFile);
			if (!fDir)
			{
				lFile.clear();
				fFile.clear();
				sFile.clear();
				Memo1->Lines->Add("업로드 실패 : 폴더내 파일을 찾을 수 없음("+ListView3->Items->Item[i]->Caption+")");
				continue;
			}
			for (unsigned int i = 0; i < lFile.size(); i++)
			{
				if(FtpConn->downCancel == 1) break;
				if(!FileExists(lFile.at(i)))
				{
					dl = FtpConn->func_fileDownLoad(lFile.at(i), fFile.at(i), sFile.at(i));
					if (dl) downC++;
					else nDownC++;
					FtpConn->downCount++;
					StatusBar1->SimpleText = "다운로드 중...("+IntToStr(downC)+")";
					Application->ProcessMessages();
				}
				else nDownC++;
			}
			lFile.clear();
			fFile.clear();
			sFile.clear();
			Application->ProcessMessages();
		}

	}
	FtpConn->Close();
	FtpConn->downCancel = 0;
	ListView2->Enabled = true;
	TreeView2->Enabled = true;
	createDirButton->Enabled = true;
	lFile.clear();
	fFile.clear();
	sFile.clear();
	Memo1->Lines->Add("다운로드 폴더: "+s_PastDownPath);
	if (downC == 0)
		Memo1->Lines->Add("다운로드 실패");
	else
	{
		Memo1->Lines->Add("다운로드 완료(성공:" + IntToStr(downC) + ", 실패:"+IntToStr(nDownC)+")");
	}
	StatusBar1->SimpleText = "";
}
//------------------------------------------------------------------------------
// ftp리스트 팝업, 파일 삭제
void __fastcall TForm1::lv2deleteClick(TObject *Sender)
{
	if(func_ftpConnState()) return;
	if(Application->MessageBoxA(L"선택한 파일(폴더)를 삭제하시겠습니까?\r\n삭제한 파일은 복구할 수 없습니다.",L"삭제",
								1|MB_ICONQUESTION) == 2)return;
	Memo1->Lines->Add("파일(폴더) 삭제 중...");
	int count = 0;
	bool fdf = false;
	bool fdd = false;
	int fdfC = 0;
	int fddC = 0;
	ListBox1->Clear();
	ListBox2->Clear();
	for (int i = 0; i < ListView2->Items->Count; i++)
	{
		if (ListView2->Items->Item[i]->Selected)
		{
			ListBox1->Items->Add(ListView2->Items->Item[i]->Caption);
			ListBox2->Items->Add
				(ListView2->Items->Item[i]->SubItems->Strings[1]);
			count++;
		}
	}
	for (int i = 0; i < count; i++)
	{
		if (ListBox2->Items->Strings[i] == "파  일")
		{
			fdf = FtpConn->func_deleteFile
				(PathBox->Text + "/" + ListBox1->Items->Strings[i]);
			if (fdf) {
				fdfC++;
			}
			StatusBar1->SimpleText = "삭제 중...("+IntToStr(fdfC)+")";
			Application->ProcessMessages();
		}
		else if (ListBox2->Items->Strings[i] == "폴  더")
		{
			vector<String>ftpFile;
			vector<String>dirFile;
			FtpConn->func_ftpdeleteFiles
				(PathBox->Text + "/" + ListBox1->Items->Strings[i],
				&dirFile, &ftpFile);
			for (unsigned int i = 0; i < ftpFile.size(); i++)
			{
				fdf = FtpConn->func_deleteFile(ftpFile.at(i));
				if (fdf)
					fdfC++;
				StatusBar1->SimpleText = "삭제 중...("+IntToStr(fdfC)+")";
				Application->ProcessMessages();
			}
			unsigned int size = dirFile.size();
			for (unsigned int i = 0; i < size; i++)
			{
				fdd = FtpConn->func_deleteDir(dirFile.back());
				if (fdd) {
					fddC++;
				}
				dirFile.pop_back();
			}
			ftpFile.clear();
			dirFile.clear();
		}
	}
	func_ftpListViewAdd(PathBox->Text);
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
	Memo1->Lines->Add("선택파일(폴더) 삭제 (파일:" + IntToStr(fdfC) + ", 폴더:"+IntToStr(fddC)+")");
	StatusBar1->SimpleText = "";
}
// ---------------------------------------------------------------------------
// 상단메뉴 보기 큰아이콘 보기
void __fastcall TForm1::N4Click(TObject *Sender) {
	ListView2->ViewStyle = (TViewStyle)0;
	N4->Checked = true;
}
// ---------------------------------------------------------------------------
// 상단메뉴 보기 자세히 보기
void __fastcall TForm1::N6Click(TObject *Sender) {
	ListView2->ViewStyle = (TViewStyle)3;
	N6->Checked = true;
}
// ---------------------------------------------------------------------------
//ftp서버 새폴더 생성
void __fastcall TForm1::lv2CreateDirClick(TObject *Sender) {
	if(func_ftpConnState()) return;
	int birdDirCount = 0;
	AnsiString Path, birdDir;
	bool cd = false;
	do{
		if(birdDirCount == 0)
		{
			birdDir = "새 폴더";
			Path = ((AnsiString)PathBox->Text)+"/"+birdDir;
		}
		else
		{
			birdDir = "새 폴더("+IntToStr(birdDirCount)+")";
			Path = ((AnsiString)PathBox->Text)+"/"+birdDir;
		}
		if(EUCKR1->Checked)
		{
			cd = FtpCreateDirectoryA(FtpConn->HFtp_Connection, (Path).c_str());
		}
		else if(UTF81->Checked)
		{
			cd = FtpCreateDirectoryA(FtpConn->HFtp_Connection, FtpConn->ANSIToUTF8(Path.c_str()).c_str());
        }

		birdDirCount++;
		if(birdDirCount == 50)
		{
			Application->MessageBoxA(L"새폴더 생성을 실패하였습니다.",L"오류",0 | MB_ICONWARNING);
			return;
		}
	}while(!cd);
    func_ftpListViewAdd(PathBox->Text);
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
	for(int i=0;i<ListView2->Items->Count;i++)
	{
		if (ListView2->Items->Item[i]->Caption == birdDir) {
			ListView2->Items->Item[i]->Selected = true;
			ListView2->Selected->EditCaption();
			break;
		}
	}

}
//---------------------------------------------------------------------------
//팝업창 중 이름바꾸기 클릭
void __fastcall TForm1::lv2RenameClick(TObject *Sender)
{
	ListView2->Selected->EditCaption();
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//ftp 리스트 선택에 따른 팝업 Enable
void __fastcall TForm1::ListView2ContextPopup(TObject *Sender,
	TPoint &MousePos, bool &Handled)

{
	if (ListView2->Selected)
	{
		lv2download->Enabled = true;
		lv2delete->Enabled = true;
		lv2copyFile->Enabled = true;
		lv2CreateDir->Enabled = false;
		lv2Rename->Enabled = true;
		downloadButton->Enabled = true;
		deleteButton->Enabled = true;
		renameButton->Enabled = true;
	}
	else
	{
		lv2download->Enabled = false;
		lv2delete->Enabled = false;
		lv2copyFile->Enabled = false;
		lv2CreateDir->Enabled = true;
		lv2Rename->Enabled = false;
		downloadButton->Enabled = false;
		deleteButton->Enabled = false;
		renameButton->Enabled = false;
	}
	if(copyFileList->Items->Count == 0) lv2pasteFile->Enabled = false;
	else lv2pasteFile->Enabled = true;
}
// ---------------------------------------------------------------------------
//탐색기 열기
void __fastcall TForm1::N1Click(TObject *Sender)
{
	char buffer[MAX_PATH];
	::SHGetSpecialFolderPath(NULL, buffer, CSIDL_DESKTOPDIRECTORY, TRUE);
	ShellExecute(Application->Handle,"open","explorer", buffer, NULL, SW_SHOW);
}
//------------------------------------------------------------------------------
//------------------------------드래그 및 버튼 클릭-----------------------------
//------------------------------------------------------------------------------
//파일(폴더) 드래그
void __fastcall TForm1::ListView2DragOver(TObject *Sender, TObject *Source, int X,
		  int Y, TDragState State, bool &Accept)
{
	if(dynamic_cast<TListView *>(Source) == NULL) return;
	if(!moveFileSelect)
	{
		moveFileList->Items->Clear();
		for(int i=0;i<ListView2->Items->Count;i++)    //선택된 리스트 개수
		{
			if(ListView2->Items->Item[i]->Selected)
			{
				moveFileList->Items->Add(ListView2->Items->Item[i]->Caption);
				moveSelected++;
			}
		}
		moveFileSelect = true;
		if(moveSelected>1) ListView2->DragCursor = crMultiDrag;  //선택된게 다중이면
		else ListView2->DragCursor = crDrag;               //하나만 선택하면
	}
	TListItem *SelItem = ListView2->GetItemAt(X,Y);
	if(SelItem == NULL) return;
	if(tarSelected == 0)
	{
		SelectedName = SelItem->Caption;
		tarSelected = 1;
	}
	for(int i=0;i<moveFileList->Items->Count;i++)
	{
		if(SelItem->Caption == moveFileList->Items->Strings[i])
		{
			SelItem = NULL;
			delete SelItem;
			return;
        }
    }
	if(SelItem->Caption != SelectedName)
	{
		for(int i=0;i<ListView2->Items->Count;i++)    //선택된 리스트 개수
		{
			if(ListView2->Items->Item[i]->Caption == SelectedName)
			{
				ListView2->GetItemAt(X,Y)->Selected = true;
				ListView2->Items->Item[i]->Selected = false;
				SelectedName = ListView2->GetItemAt(X,Y)->Caption;
				break;
			}
		}
	}
	SelItem = NULL;
	delete SelItem;

}
//---------------------------------------------------------------------------
//드래그 된 파일(폴더) 이동
void __fastcall TForm1::ListView2DragDrop(TObject *Sender, TObject *Source, int X,
          int Y)
{
	int cdc = 0;
	TListItem *TarItem = ListView2->GetItemAt(X,Y);
	if(TarItem == NULL)
	{
		moveFileSelect = false;
		moveSelected = 0;
		tarSelected  = 0;
		return;
	}
	AnsiString target = TarItem->Caption;
	if(target == "상위폴더로 이동")
	{
        target = "..";
    }
	AnsiString fileK  = TarItem->SubItems->Strings[1];
	if(fileK == "파  일")
	{
		moveFileSelect = false;
		moveSelected = 0;
		tarSelected  = 0;
		return;
	}
	Memo1->Lines->Add("파일(폴더) 이동 중...");
	bool cd;

	AnsiString movePath = PathBox->Text+"/"+target+"/";

	for(int i=0;i<moveFileList->Count;i++)
	{
		AnsiString pathResult = movePath+moveFileList->Items->Strings[i];
		AnsiString existFile  = moveFileList->Items->Strings[i];
		if(EUCKR1->Checked)
		{
			cd = FtpRenameFile(FtpConn->HFtp_Connection, existFile.c_str(), pathResult.c_str());
		}
		else if(UTF81->Checked)
		{
			cd = FtpRenameFile(FtpConn->HFtp_Connection, FtpConn->ANSIToUTF8(existFile.c_str()).c_str(),
									FtpConn->ANSIToUTF8(pathResult.c_str()).c_str());
		}

		if(cd) cdc++;
	}
	TarItem = NULL;
	delete TarItem;
	moveFileSelect = false;
	moveSelected = 0;
	tarSelected  = 0;
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
	func_ftpListViewAdd(PathBox->Text);
	if(cdc >0) Memo1->Lines->Add("파일(폴더) 이동 완료.("+IntToStr(cdc)+")");
	else Memo1->Lines->Add("파일(폴더) 이동 실패.");
	for(int i=0;i<ListView2->Items->Count;i++)    //선택된 리스트 개수
	{
		if(ListView2->Items->Item[i]->Selected)
		{
			ListView2->Items->Item[i]->Selected = false;
		}
	}
}
//---------------------------------------------------------------------------
//이름 변경 시
void __fastcall TForm1::ListView2Edited(TObject *Sender, TListItem *Item, UnicodeString &S)
{
	if(func_ftpConnState()) return;
	bool rn = false;
	int fdCount = 0;
	ListView2->Selected->CancelEdit();
	AnsiString exPath = ListView2->Selected->Caption;
	AnsiString nePath = S;
	if(nePath == "")
	{
		Application->MessageBoxA(L"변경할 이름을 입력하세요.",L"오류",0| MB_ICONWARNING);
		ListView2->Selected->Caption = exPath;
		return;
    }
	do{
		if(fdCount == 0) nePath = S;
		else if(S.Pos(".") == 0) nePath = S+"("+IntToStr(fdCount)+")";
		else nePath = S.SubString(1,S.Pos(".")-1)+"("+IntToStr(fdCount)+")"+S.SubString(S.Pos("."), S.Length()-S.Pos(".")+1);
		if(EUCKR1->Checked)
			rn = FtpRenameFileA(FtpConn->HFtp_Connection, exPath.c_str(), nePath.c_str());
		else if(UTF81->Checked)
			rn = FtpRenameFileA(FtpConn->HFtp_Connection, FtpConn->ANSIToUTF8(exPath.c_str()).c_str(),
										FtpConn->ANSIToUTF8(nePath.c_str()).c_str());
		fdCount++;
		if(fdCount == 50)
		{
			Application->MessageBoxA(L"이름 변경을 실패하였습니다.",L"오류",0 | MB_ICONWARNING);
			return;
		}
	}while(!rn);
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
	func_ftpListViewAdd(PathBox->Text);
}
//---------------------------------------------------------------------------
//F2키 입력 시 이름변경
void __fastcall TForm1::ListView2KeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key == 113)
	{
		if(ListView2->Selected) ListView2->Selected->EditCaption();
	}
	if(Key == 46)
	{
		TObject *S;
		if(ListView2->Selected) lv2deleteClick(S);
	}
	if(GetAsyncKeyState(17) & 0x8000)
	{
		if(Key == 67)
		{
			TObject *S;
			lv2copyFileClick(S);
		}
		else if(Key == 86)
		{
			TObject *S;
			lv2pasteFileClick(S);
		}
		else if(Key == 65)
		{
			TObject *S;
			lv2allFileClick(S);
		}
	}
}
//---------------------------------------------------------------------------
//컬럼 클릭 시 오름차순 내림차순 설정
void __fastcall TForm1::ListView2ColumnClick(TObject *Sender, TListColumn *Column)
{
	if(func_ftpConnState()) return;
	if(Column->Index == 2) return;
	if(SortByColumn == Column->Index) SortOrder *= -1;
	else SortOrder = 1;
	SortByColumn = Column->Index;
	sort(SortOrder, SortByColumn);
	ListView2->Columns->Items[0]->Caption = listColumn1;
	ListView2->Columns->Items[1]->Caption = listColumn2;
	ListView2->Columns->Items[2]->Caption = listColumn3;
	ListView2->Columns->Items[3]->Caption = listColumn4;
	switch(SortByColumn)
	{
	case 0:
		if(SortOrder == -1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn1+"(▼)";
		else if(SortOrder == 1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn1+"(▲)";
	break;
	case 1:
		if(SortOrder == 1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn2+"(▼)";
		else if(SortOrder == -1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn2+"(▲)";
	break;
	case 3:
		if(SortOrder == 1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn4+"(▼)";
		else if(SortOrder == -1) ListView2->Columns->Items[SortByColumn]->Caption = listColumn4+"(▲)";
	break;
	}
}
//---------------------------------------------------------------------------
//컬럼 클릭에 따른 정렬
void __fastcall TForm1::sort(int asc, int what)
{
	TStringList *nameList = new TStringList;
	nameList->Clear();
	TStringList *sizeList = new TStringList;
	sizeList->Clear();
	TStringList *typeList = new TStringList;
	typeList->Clear();
	TStringList *dateList = new TStringList;
	dateList->Clear();
	TStringList *iconList = new TStringList;
	iconList->Clear();
	for(UINT i=0;i<FtpConn->ftpfileValue.fileName.size();i++)   //struct 내용을 StringList로 복사
	{
		nameList->Add(FtpConn->ftpfileValue.fileName.at(i));
		AnsiString comNo = StringReplace(FtpConn->ftpfileValue.fileSize.at(i),",","",TReplaceFlags() << rfReplaceAll);
		sizeList->Add(FtpConn->ftpfileValue.fileSize.at(i));
		typeList->Add(FtpConn->ftpfileValue.fileType.at(i));
		dateList->Add(FtpConn->ftpfileValue.fileTime.at(i));
		iconList->Add(FtpConn->ftpfileValue.fileIcon.at(i));
	}
	for(int i=0;i<typeList->Count;i++)                         //폴더, 파일 정렬
	{
		for(int j=i+1;j<typeList->Count;j++)
		{
			bool bs = false;
			if(typeList->Strings[i] == "") continue;
			bs = typeList->Strings[i] < typeList->Strings[j];
			if(bs)
			{
				AnsiString temp = nameList->Strings[i];
				nameList->Strings[i] = nameList->Strings[j];
				nameList->Strings[j] = temp;
				temp = sizeList->Strings[i];
				sizeList->Strings[i] = sizeList->Strings[j];
				sizeList->Strings[j] = temp;
				temp = typeList->Strings[i];
				typeList->Strings[i] = typeList->Strings[j];
				typeList->Strings[j] = temp;
				temp = dateList->Strings[i];
				dateList->Strings[i] = dateList->Strings[j];
				dateList->Strings[j] = temp;
				temp = iconList->Strings[i];
				iconList->Strings[i] = iconList->Strings[j];
				iconList->Strings[j] = temp;
			}
		}
	}
	if(what == 0){
		for(int i=0;i<nameList->Count;i++)                    //이름 정렬
		{
			for(int j=i+1;j<nameList->Count;j++)
			{
				bool bs = false;
				if(typeList->Strings[i] == "") continue;
				int asd = 0;
				asd = StrCmpLogicalW(nameList->Strings[i].c_str(), nameList->Strings[j].c_str());
				if(asc == 1)
				{
					if(asd == 1) bs = true;
					else if(asd == -1) bs = false;
					else continue;
				}
				else if(asc == -1){
					if(asd == 1) bs = false;
					else if(asd == -1) bs = true;
					else continue;
				}
				if(bs)
				{
					if(typeList->Strings[i] != typeList->Strings[j]) continue;
					AnsiString temp = nameList->Strings[i];
					nameList->Strings[i] = nameList->Strings[j];
					nameList->Strings[j] = temp;
					temp = sizeList->Strings[i];
					sizeList->Strings[i] = sizeList->Strings[j];
					sizeList->Strings[j] = temp;
					temp = typeList->Strings[i];
					typeList->Strings[i] = typeList->Strings[j];
					typeList->Strings[j] = temp;
					temp = dateList->Strings[i];
					dateList->Strings[i] = dateList->Strings[j];
					dateList->Strings[j] = temp;
					temp = iconList->Strings[i];
					iconList->Strings[i] = iconList->Strings[j];
					iconList->Strings[j] = temp;
				}
			}
		}
	}
	if(what == 1){                                             //크기정렬
		for(int i=0;i<sizeList->Count;i++)
		{
			for(int j=i+1;j<sizeList->Count;j++)
			{
				bool bs = false;
				if(typeList->Strings[i] == "") continue;
				if(asc == 1)
					bs = StrToInt64Def(sizeList->Strings[i], 0) < StrToInt64Def(sizeList->Strings[j], 0);
				else if(asc == -1)
					bs = StrToInt64Def(sizeList->Strings[i], 0) > StrToInt64Def(sizeList->Strings[j], 0);
				if(bs)
				{
					if(typeList->Strings[i] != typeList->Strings[j]) continue;
					AnsiString temp = nameList->Strings[i];
					nameList->Strings[i] = nameList->Strings[j];
					nameList->Strings[j] = temp;
					temp = sizeList->Strings[i];
					sizeList->Strings[i] = sizeList->Strings[j];
					sizeList->Strings[j] = temp;
					temp = typeList->Strings[i];
					typeList->Strings[i] = typeList->Strings[j];
					typeList->Strings[j] = temp;
					temp = dateList->Strings[i];
					dateList->Strings[i] = dateList->Strings[j];
					dateList->Strings[j] = temp;
					temp = iconList->Strings[i];
					iconList->Strings[i] = iconList->Strings[j];
					iconList->Strings[j] = temp;
				}
			}
		}
	}
	if(what == 3){                                         //날짜정렬
		for(int i=0;i<dateList->Count;i++)
		{
			for(int j=i+1;j<dateList->Count;j++)
			{
				bool bs = false;
				if(typeList->Strings[i] == "") continue;
				if(asc == 1)
					bs = dateList->Strings[i] < dateList->Strings[j];
				else if(asc == -1)
					bs = dateList->Strings[i] > dateList->Strings[j];
				if(bs)
				{
					if(typeList->Strings[i] != typeList->Strings[j]) continue;
					AnsiString temp = nameList->Strings[i];
					nameList->Strings[i] = nameList->Strings[j];
					nameList->Strings[j] = temp;
					temp = sizeList->Strings[i];
					sizeList->Strings[i] = sizeList->Strings[j];
					sizeList->Strings[j] = temp;
					temp = typeList->Strings[i];
					typeList->Strings[i] = typeList->Strings[j];
					typeList->Strings[j] = temp;
					temp = dateList->Strings[i];
					dateList->Strings[i] = dateList->Strings[j];
					dateList->Strings[j] = temp;
					temp = iconList->Strings[i];
					iconList->Strings[i] = iconList->Strings[j];
					iconList->Strings[j] = temp;
				}
			}
		}
	}
	FtpConn->ftpfileValue.fileName.clear();            //struct 초기화
	FtpConn->ftpfileValue.fileSize.clear();
	FtpConn->ftpfileValue.fileType.clear();
	FtpConn->ftpfileValue.fileTime.clear();
	FtpConn->ftpfileValue.fileIcon.clear();
	for(int i=0;i<nameList->Count;i++)                                  //struct에 정렬된 StringList 이동
	{
		FtpConn->ftpfileValue.fileName.push_back(nameList->Strings[i]);
		FtpConn->ftpfileValue.fileSize.push_back(sizeList->Strings[i]);
		FtpConn->ftpfileValue.fileType.push_back(typeList->Strings[i]);
		FtpConn->ftpfileValue.fileTime.push_back(dateList->Strings[i]);
		FtpConn->ftpfileValue.fileIcon.push_back(StrToInt(((AnsiString)iconList->Strings[i]).c_str()));
	}
	nameList->Clear();
	delete nameList;
	sizeList->Clear();
	delete sizeList;
	typeList->Clear();
	delete typeList;
	dateList->Clear();
	delete dateList;
	iconList->Clear();
	delete iconList;
	ListView2->Items->Count = FtpConn->ftpfileValue.fileName.size();
	ListView2->Refresh();
}
//---------------------------------------------------------------------------
//PORT에디트 엔터 클릭 시 연결버튼 클릭
void __fastcall TForm1::Edit4KeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key == VK_RETURN)
	{
		TObject *S;
		Button1Click(S);
	}
}
//---------------------------------------------------------------------------
//PW에디트 엔터 클릭 시 연결버튼 클릭
void __fastcall TForm1::Edit3KeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if(Key == VK_RETURN)
	{
		TObject *S;
		Button1Click(S);
	}
}
//------------------------------------------------------------------------------
//when click listview2
void __fastcall TForm1::ListView2Click(TObject *Sender)
{
	if (ListView2->Selected)
	{
		downloadButton->Enabled = true;
		deleteButton->Enabled = true;
		renameButton->Enabled = true;
		createDirButton->Enabled = false;
		if(func_ftpConnState()) return;
		if(preViewPanel->Visible == true)
		{
			if (ListView2->Selected->SubItems->Strings[1] == "파  일")
			{
				AnsiString s_FileName = ListView2->Selected->Caption;
				AnsiString s_FileSize = ListView2->Selected->SubItems->Strings[0];
				s_FileSize = StringReplace(s_FileSize, ",", "",TReplaceFlags() << rfReplaceAll);
				ULONGLONG s_FileSizeU = StrToInt64Def(s_FileSize, 0);
				if(!s_FileSizeU) return;
				TWICImage *pWIC = new TWICImage();
				AnsiString s_extFile = LowerCase(ExtractFileExt(s_FileName));
				if(s_extFile!=".jpg" && s_extFile!=".png" && s_extFile!=".tif" &&
				   s_extFile!=".bmp" && s_extFile!=".gif"){
					preViewLabel->Caption = "지원하지 않는 포맷입니다";
					Image1->Picture = NULL;
					delete pWIC;
					pWIC = NULL;
					return;
				}
				b_downBool = true;
				AnsiString s_localName = "v_"+s_FileName;
				FtpConn->func_fileReview(s_FileName, s_localName, s_FileSizeU);
				b_downBool = false;
				AnsiString ExeFiles = ExtractFilePath(Application->ExeName)+"tmpDir\\"+s_localName;
				try{
					preViewLabel->Caption = "";
					pWIC->LoadFromFile(ExeFiles);
					Image1->Picture->Assign(pWIC);
					delete pWIC;
					pWIC = NULL;
				}catch(...){
					preViewLabel->Caption = "지원하지 않는 포맷입니다";
					Image1->Picture = NULL;
					delete pWIC;
					pWIC = NULL;
				}
				return;
			}
		}
		StatusBar1->SimpleText = "선택한 파일 : "+IntToStr(ListView2->SelCount) +"개";
	}
	else
	{
		downloadButton->Enabled = false;
		deleteButton->Enabled = false;
		renameButton->Enabled = false;
		createDirButton->Enabled = true;
		int s_allCount = ListView2->Items->Count-1;
		if(PathBox->Text == "/") s_allCount = s_allCount+1;
		StatusBar1->SimpleText = "전체 파일 : "+IntToStr(s_allCount) +"개";
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::downloadButtonClick(TObject *Sender)
{
	TObject *ss;
	lv2downloadClick(ss);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::deleteButtonClick(TObject *Sender)
{
	TObject *ss;
	lv2deleteClick(ss);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::createDirButtonClick(TObject *Sender)
{
	TObject *ss;
	lv2CreateDirClick(ss);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::renameButtonClick(TObject *Sender)
{
	TObject *ss;
	lv2RenameClick(ss);
}
//---------------------------------------------------------------------------
//뒤로가기 버튼 mouseDown 시 화살표 모양 변경
void __fastcall TForm1::SpeedButton1MouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y)
{
	try{
	SpeedButton1->Glyph->LoadFromFile(ExtractFilePath(Application->ExeName)+"\\Image\\push_button.bmp");
	}catch(...){};
}
//---------------------------------------------------------------------------
//뒤로가기 버튼 mouseUp 시 화살표 모양 변경
void __fastcall TForm1::SpeedButton1MouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y)
{
	try{
	SpeedButton1->Glyph->LoadFromFile(ExtractFilePath(Application->ExeName)+"\\Image\\pull_button.bmp");
	}catch(...){};

	if(func_ftpConnState()) return;
	if(PathBox->Text == "/") return;
	Memo1->Lines->Add("상위 폴더로 이동 중...");
	TTreeNode *treeSel;
	AnsiString Path = "..";
	treeSel = TreeView2->Selected->Parent;
	TreeView2->Selected = treeSel;
	treeSel->DeleteChildren();
	func_ftpTreeView(Path, treeSel);
	TreeView2->Selected = treeSel;
	PathBox->Text = ftpGetAncestor(TreeView2->Selected);
	func_ftpListViewAdd(PathBox->Text);
	Memo1->Lines->Add("이동 완료.");
	int s_allCount = ListView2->Items->Count-1;
	if(PathBox->Text == "/") s_allCount = s_allCount+1;
	StatusBar1->SimpleText = "전체 파일 : "+IntToStr(s_allCount) +"개";
}
//---------------------------------------------------------------------------
//-----------------------------화면->화면편집--------------------------------
//---------------------------------------------------------------------------
//서버 입력화면
void __fastcall TForm1::infoMenuClick(TObject *Sender)
{
	if(infoMenu->Checked)
	{
		InfoPanel->Visible = false;
		infoMenu->Checked = false;
	}
	else
	{
		InfoPanel->Visible = true;
		infoMenu->Checked = true;
	}
}
//---------------------------------------------------------------------------
//메모화면
void __fastcall TForm1::memoMenuClick(TObject *Sender)
{
	if(memoMenu->Checked)
	{
		Memo1->Visible = false;
		memoMenu->Checked = false;
	}
	else
	{
		Memo1->Visible = true;
		memoMenu->Checked = true;
	}
}
//---------------------------------------------------------------------------
//트리뷰화면
void __fastcall TForm1::treeViewMenuClick(TObject *Sender)
{
	if(treeViewMenu->Checked)
	{
		TreePanel->Visible = false;
		treeViewMenu->Checked = false;
	}
	else
	{
		TreePanel->Visible = true;
		treeViewMenu->Checked = true;
	}
}
//---------------------------------------------------------------------------
//종료메뉴
void __fastcall TForm1::exitMenuClick(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------
//마우스 드래그 후 올릴 때
void __fastcall TForm1::ListView2MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
	swsw = 0;
	if (ListView2->Selected)
	{
		downloadButton->Enabled = true;
		deleteButton->Enabled = true;
		renameButton->Enabled = true;
	}
	else
	{
		downloadButton->Enabled = false;
		deleteButton->Enabled = false;
		renameButton->Enabled = false;
	}
}
//------------------------------------------------------------------------------
//-----------------------------ftp 내 파일 복사, 붙여넣기-----------------------
//------------------------------------------------------------------------------
//선택파일 복사(ListView에 저장)
void __fastcall TForm1::lv2copyFileClick(TObject *Sender)
{
	copyFileList->Items->Clear();
	int count = 0;
	for(int i=0;i<ListView2->Items->Count;i++)    //선택된 리스트 개수
	{
		if(ListView2->Items->Item[i]->Selected)
		{
			copyFileList->Items->Add();
			copyFileList->Items->Item[count]->Caption = ListView2->Items->Item[i]->Caption;
			copyFileList->Items->Item[count]->SubItems->Add(ListView2->Items->Item[i]->SubItems->Strings[1]);
			copyFileList->Items->Item[count]->SubItems->Add(ListView2->Items->Item[i]->SubItems->Strings[0]);
			copyFileList->Items->Item[count]->SubItems->Strings[1] = StringReplace
				(copyFileList->Items->Item[count]->SubItems->Strings[1], ",", "",
				TReplaceFlags() << rfReplaceAll);
			copyFileList->Items->Item[count]->SubItems->Add(PathBox->Text+"/");
			count++;
		}
	}
	Memo1->Lines->Add("선택 파일이 클립보드에 저장 됨("+IntToStr(count)+")");
}
//---------------------------------------------------------------------------
//파일 붙여넣기
void __fastcall TForm1::lv2pasteFileClick(TObject *Sender)
{
	if(copyFileList->Items->Count == 0) return;
	Memo1->Lines->Add("복사 준비 중...");
	Application->ProcessMessages();
	ListView2->Enabled = false;
	TreeView2->Enabled = false;
	downloadButton->Enabled = false;
	deleteButton->Enabled = false;
	createDirButton->Enabled = false;
	renameButton->Enabled = false;
	int count = 0;
	int copyC = 0;
	int nCopyC = 0;
	vector<String>exFile;
	vector<String>cpFile;
	vector<__int64>sFile;
	FtpConn->downCountFull = 0;
	FtpConn->ProgressBar2->Position = 0;
	for(int i=0;i<copyFileList->Items->Count;i++)
	{
		if(copyFileList->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			FtpConn->downCountFull++;
		}
		else if(copyFileList->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			FtpConn->func_ftpFileCountF(copyFileList->Items->Item[i]->SubItems->Strings[2]+copyFileList->Items->Item[i]->Caption);
		}
	}
	FtpConn->ProgressBar2->Max = FtpConn->downCountFull;
	FtpConn->downCount = 0;
	Memo1->Lines->Add("복사 시작");
	FtpConn->Show();
	Application->ProcessMessages();
	AnsiString IP = Edit1->Text;
	int PORT = StrToInt(((AnsiString)Edit4->Text).c_str());
	AnsiString ID = Edit2->Text;
	AnsiString PW = Edit3->Text;
	FtpConn->nFtp_Session = ::InternetOpen("FTP CLIENT TEST", INTERNET_OPEN_TYPE_DIRECT, NULL, 0, 0);   //세선연결
	FtpConn->nFtp_Connection = ::InternetConnect(FtpConn->nFtp_Session,IP.c_str(),PORT,ID.c_str(),PW.c_str(),INTERNET_SERVICE_FTP,mode,0);
	for(int i = 0; i<copyFileList->Items->Count; i++)
	{
		if(FtpConn->downCancel == 1) break;
		for(int j=0;j<ListView2->Items->Count;j++)    //선택된 리스트 개수
		{
			if(ListView2->Items->Item[j]->Caption == copyFileList->Items->Item[i]->Caption) count++;
		}
		if(count>0){count = 0; nCopyC++; continue;}
		bool dl = false;
		AnsiString exPath =  copyFileList->Items->Item[i]->SubItems->Strings[2]+copyFileList->Items->Item[i]->Caption;
		AnsiString newPath = PathBox->Text+"/"+copyFileList->Items->Item[i]->Caption;

		if (copyFileList->Items->Item[i]->SubItems->Strings[0] == "파  일")
		{
			__int64 size = StrToInt64(copyFileList->Items->Item[i]->SubItems->Strings[1]);
			dl = FtpConn->func_fileCopy(exPath, newPath, size);
			if(dl) copyC++;
			else nCopyC++;
			FtpConn->downCount++;
			StatusBar1->SimpleText = "복사 중...("+IntToStr(copyC)+")";
			Application->ProcessMessages();
		}
		else if (copyFileList->Items->Item[i]->SubItems->Strings[0] == "폴  더")
		{
			bool fDir = FtpConn->func_ftpFindCopyFiles(newPath,exPath, &exFile, &cpFile, &sFile);
			if (!fDir)
			{
				exFile.clear();
				cpFile.clear();
				sFile.clear();
				Memo1->Lines->Add("업로드 실패 : 폴더 내 파일을 찾을 수 없음("+exPath+")");
				continue;
			}
			for (unsigned int i = 0; i < exFile.size(); i++)
			{
				if(FtpConn->downCancel == 1) break;
				dl = FtpConn->func_fileCopy(exFile.at(i), cpFile.at(i), sFile.at(i));
				if (dl) copyC++;
				else nCopyC++;
				FtpConn->downCount++;
				StatusBar1->SimpleText = "붙여넣기 중...("+IntToStr(copyC)+")";
				Application->ProcessMessages();
			}
			exFile.clear();
			cpFile.clear();
			sFile.clear();
			Application->ProcessMessages();
		}
	}
	FtpConn->Close();
	FtpConn->downCancel = 0;
	ListView2->Enabled = true;
	TreeView2->Enabled = true;
	createDirButton->Enabled = true;
	exFile.clear();
	cpFile.clear();
	sFile.clear();
	InternetCloseHandle(FtpConn->nFtp_Connection);
	InternetCloseHandle(FtpConn->nFtp_Session);
	if (copyC == 0)
		Memo1->Lines->Add("복사 실패");
	else
		Memo1->Lines->Add("복사 완료(성공:" + IntToStr(copyC) + ", 실패:"+IntToStr(nCopyC)+")");
	StatusBar1->SimpleText = "";
	func_ftpListViewAdd(PathBox->Text);
	TreeView2->Selected->DeleteChildren();
	func_ftpTreeView(PathBox->Text, TreeView2->Selected);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::EUCKR1Click(TObject *Sender)
{
	EUCKR1->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::UTF81Click(TObject *Sender)
{
	UTF81->Checked = true;
}

void __fastcall TForm1::preViewButtonClick(TObject *Sender)
{
	if(preViewButton->Caption =="미리보기")
	{
		this->Width = this->Width+300;
		preViewPanel->Visible = true;
		Splitter3->Visible = true;
		preViewButton->Caption = "닫기";
	}
	else if(preViewButton->Caption == "닫기")
	{
		this->Width = this->Width-300;
		preViewPanel->Visible = false;
		Splitter3->Visible = false;
		preViewButton->Caption = "미리보기";
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::preViewPanelResize(TObject *Sender)
{
	preViewLabel->Left = preViewPanel->Width/3;
	preViewLabel->Top = preViewPanel->Height/2;
	preViewLabel->Width = preViewPanel->Width;
	preViewLabel->Height = 50;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
	if(b_downBool) return;
	else Timer1->Enabled = false;
	{
		AnsiString s_FileName = ListView2->Selected->Caption;
		AnsiString s_FileSize = ListView2->Selected->SubItems->Strings[0];
		s_FileSize = StringReplace(s_FileSize, ",", "",TReplaceFlags() << rfReplaceAll);
		ULONGLONG s_FileSizeU = StrToInt64Def(s_FileSize, 0);
		if(!s_FileSizeU) return;
		FtpConn->Show();
		FtpConn->func_fileReview(s_FileName, s_FileName, s_FileSizeU);
		FtpConn->Close();
		AnsiString ExeFiles = ExtractFilePath(Application->ExeName)+"tmpDir\\"+s_FileName;
		SHELLEXECUTEINFO m_seInfo;
		ZeroMemory(&m_seInfo, sizeof(SHELLEXECUTEINFO));
		m_seInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		m_seInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		m_seInfo.lpFile = ExeFiles.c_str();
		m_seInfo.nShow = SW_SHOW;
		::ShellExecuteEx(&m_seInfo);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::lv2allFileClick(TObject *Sender)
{
	ListView2->SelectAll();
	if(PathBox->Text != "/") ListView2->Items->Item[0]->Selected = false;
	StatusBar1->SimpleText = "선택한 파일 : "+IntToStr(ListView2->SelCount) +"개";
}
//---------------------------------------------------------------------------


