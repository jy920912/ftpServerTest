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
		vector<String> 		fileName;      //���� �̸�
		vector<String> 		fileSize;      //���� ũ��
		vector<String> 		fileType;      //���� or ����
		vector<String> 		fileTime;      //���� ���� �ð�
		vector<int>    		fileIcon;      //���� ������
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
	void __fastcall func_ftpInit();        //ftp �ʱ�ȭ(���� ����)
	bool __fastcall func_ftpConnect(AnsiString IP, AnsiString PORT, AnsiString ID, AnsiString PW, int MODE);//ftp ����(passive, active���)
	void __fastcall func_initStruct();     //ftp ����ü(vector) �ʱ�ȭ
	bool __fastcall func_GetFTPFileList(AnsiString Dir, FTPFILEVALUE *fileValue); //���� ���丮 �� ����(����) �˻�
	bool __fastcall func_fileUpLoad(AnsiString fileName, AnsiString path);      //���� ���ε�
	bool __fastcall func_fileDownLoad(AnsiString fileName, AnsiString path, ULONGLONG Size); //����(����) �ٿ�ε�
	bool __fastcall func_deleteFile(AnsiString Path);                           //ftp���� ����
	bool __fastcall func_deleteDir(AnsiString Path);                            //ftp���� ����
	bool __fastcall func_ftpFindFiles(AnsiString Path, AnsiString ftpPath,      //���� �ٿ�ε� �� ���� ���� �Ϻ� ��ü �˻�
									vector<String> *v_localPath, vector<String> *v_ftpPath, vector<__int64> *v_size);
	bool __fastcall func_localFindAllFiles(AnsiString Path, AnsiString ftpPath, //���� ���ε� �� ���� ���� �Ϻ� ��ü �˻�
									vector<String> *v_localPath, vector<String> *v_ftpPath);
	bool __fastcall func_ftpdeleteFiles(AnsiString ftpPath, vector<String> *v_dirPath, vector<String> *v_ftpPath); //���� ���� �� ���� ���� �Ϻ� ��ü �˻�
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
