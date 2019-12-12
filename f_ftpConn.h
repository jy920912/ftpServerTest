//---------------------------------------------------------------------------

#ifndef f_FtpConnH
#define f_FtpConnH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <vector.h>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TFtpConn : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TPanel *Panel2;
	TLabel *Label1;
	TProgressBar *ProgressBar1;
	TPanel *Panel3;
	TLabel *Label3;
	TProgressBar *ProgressBar2;
	TLabel *Label2;
	TButton *Button1;
	TLabel *Label4;
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
	typedef struct{
		vector<String> 		fileName;      //파일 이름
		vector<String> 		fileSize;      //파일 크기
		vector<String> 		fileType;      //폴더 or 파일
		vector<String> 		fileTime;      //파일 수정 시간
		vector<int>    		fileIcon;      //파일 아이콘
	}FTPFILEVALUE;
public:		// User declarations
	HINTERNET HFtp_Session;
	HINTERNET HFtp_Connection;
	HINTERNET nFtp_Session;
	HINTERNET nFtp_Connection;
	FTPFILEVALUE ftpfileValue;
	int downCancel;
	int downCountFull;
	int downCount;
	void __fastcall func_ftpInit();        //ftp 초기화(접속 해제)
	bool __fastcall func_ftpConnect(AnsiString IP, AnsiString PORT, AnsiString ID, AnsiString PW, int MODE);//ftp 연결(passive, active모드)
	void __fastcall func_initStruct();     //ftp 구조체(vector) 초기화
	bool __fastcall func_GetFTPFileList(AnsiString Dir, FTPFILEVALUE *fileValue); //현재 디렉토리 내 파일(폴더) 검색
	bool __fastcall func_fileUpLoad(AnsiString fileName, AnsiString path);      //파일 업로드
	bool __fastcall func_fileDownLoad(AnsiString fileName, AnsiString path, ULONGLONG Size); //파일(폴더) 다운로드
	bool __fastcall func_deleteFile(AnsiString Path);                           //ftp파일 제거
	bool __fastcall func_deleteDir(AnsiString Path);                            //ftp폴더 제거
	bool __fastcall func_ftpFindFiles(AnsiString Path, AnsiString ftpPath,      //폴더 다운로드 시 선택 폴더 하부 전체 검색
									vector<String> *v_localPath, vector<String> *v_ftpPath, vector<__int64> *v_size);
	bool __fastcall func_localFindAllFiles(AnsiString Path, AnsiString ftpPath, //폴더 업로드 시 선택 폴더 하부 전체 검색
									vector<String> *v_localPath, vector<String> *v_ftpPath);
	bool __fastcall func_ftpdeleteFiles(AnsiString ftpPath, vector<String> *v_dirPath, vector<String> *v_ftpPath); //폴더 삭제 시 선택 폴더 하부 전체 검색
	bool __fastcall func_fileCopy(AnsiString existPath, AnsiString newPath, ULONGLONG Size);
	bool __fastcall func_ftpFindCopyFiles(AnsiString newPath, AnsiString exPath,
										vector<String> *v_exPath, vector<String> *v_newPath, vector<__int64> *v_size);
	bool __fastcall func_ftpFileCountF(AnsiString ftpPath);
	bool __fastcall func_localFileCountF(AnsiString Path);
	bool __fastcall func_fileReview(AnsiString fileName, AnsiString localName, ULONGLONG Size);
	AnsiString __fastcall UTF8ToANSI(const char *pszCode);
	AnsiString __fastcall ANSIToUTF8(const char * pszCode);
	__fastcall TFtpConn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFtpConn *FtpConn;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

