//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("f_ftpConn.cpp", FtpConn);
USEFORM("f_MainForm.cpp", Form1);
//---------------------------------------------------------------------------
WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	HANDLE Mutex;
	try
	{
		const char mutexname[] = "ftpConnP";
		Mutex = OpenMutex(MUTEX_ALL_ACCESS, false, mutexname);
		if(Mutex == NULL) Mutex = CreateMutex(NULL, true, mutexname);
		else return 0;
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TFtpConn), &FtpConn);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
