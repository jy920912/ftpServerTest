//---------------------------------------------------------------------------

#ifndef f_MainFormH
#define f_MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <vector.h>
#include <FileCtrl.hpp>
#include <ImgList.hpp>
#include <Grids.hpp>
#include <Outline.hpp>
#include "IdBaseComponent.hpp"
#include "IdCoder.hpp"
#include "IdCoder3to4.hpp"
#include "IdCoderMIME.hpp"
#include <Buttons.hpp>
#include <ButtonGroup.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TMenuItem *N1;
	TPanel *AllPanel;
	TPopupMenu *lv2PopUpMenu;
	TMenuItem *lv2download;
	TMenuItem *lv2delete;
	TMenuItem *N3;
	TImageList *iconBig;
	TListBox *ListBox1;
	TListBox *ListBox2;
	TMenuItem *lv2CreateDir;
	TPanel *InfoPanel;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TEdit *Edit1;
	TEdit *Edit2;
	TEdit *Edit3;
	TEdit *Edit4;
	TButton *Button1;
	TCheckBox *CheckBox1;
	TPanel *ExplorerPanel;
	TPanel *TreePanel;
	TTreeView *TreeView2;
	TPanel *ListViewPanel;
	TListView *ListView2;
	TMemo *Memo1;
	TSplitter *Splitter1;
	TPanel *Panel1;
	TEdit *PathBox;
	TPanel *Panel2;
	TListView *ListView3;
	TMenuItem *lv2Rename;
	TImageList *iconSmall;
	TIdEncoderMIME *IdEncode;
	TIdDecoderMIME *IdDecode;
	TPanel *Panel3;
	TButton *renameButton;
	TPanel *Panel4;
	TButton *downloadButton;
	TPanel *Panel5;
	TButton *deleteButton;
	TPanel *Panel6;
	TButton *preViewButton;
	TPanel *Panel7;
	TPanel *Panel8;
	TSpeedButton *SpeedButton1;
	TPanel *Panel9;
	TPanel *statusPanel;
	TStatusBar *StatusBar1;
	TMenuItem *iconView;
	TMenuItem *N4;
	TMenuItem *N6;
	TMenuItem *N5;
	TMenuItem *infoMenu;
	TMenuItem *memoMenu;
	TMenuItem *treeViewMenu;
	TMenuItem *openExplorer;
	TMenuItem *exitMenu;
	TListView *copyFileList;
	TMenuItem *lv2copyFile;
	TMenuItem *lv2pasteFile;
	TListBox *moveFileList;
	TMenuItem *N2;
	TMenuItem *EUCKR1;
	TMenuItem *UTF81;
	TPanel *preViewPanel;
	TPanel *Panel11;
	TButton *createDirButton;
	TSplitter *Splitter3;
	TLabel *preViewLabel;
	TImage *Image1;
	TTimer *Timer1;
	TSplitter *Splitter2;
	TMenuItem *N7;
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ListView2DblClick(TObject *Sender);
	void __fastcall lv2deleteClick(TObject *Sender);
	void __fastcall N4Click(TObject *Sender);
	void __fastcall N6Click(TObject *Sender);
	void __fastcall lv2CreateDirClick(TObject *Sender);
	void __fastcall ListView2ContextPopup(TObject *Sender, TPoint &MousePos, bool &Handled);
	void __fastcall WMDropFiles(TWMDropFiles &message);
	void __fastcall N1Click(TObject *Sender);
	void __fastcall ListView2DragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall ListView2DragOver(TObject *Sender, TObject *Source, int X, int Y,
          TDragState State, bool &Accept);
	void __fastcall ListView2Edited(TObject *Sender, TListItem *Item, UnicodeString &S);
	void __fastcall ListView2KeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall ListView2Data(TObject *Sender, TListItem *Item);
	void __fastcall lv2RenameClick(TObject *Sender);
	void __fastcall ListView2ColumnClick(TObject *Sender, TListColumn *Column);
	void __fastcall lv2downloadClick(TObject *Sender);
	void __fastcall Edit4KeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Edit3KeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall ListView2Click(TObject *Sender);
	void __fastcall downloadButtonClick(TObject *Sender);
	void __fastcall deleteButtonClick(TObject *Sender);
	void __fastcall createDirButtonClick(TObject *Sender);
	void __fastcall renameButtonClick(TObject *Sender);
	void __fastcall SpeedButton1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall SpeedButton1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall infoMenuClick(TObject *Sender);
	void __fastcall memoMenuClick(TObject *Sender);
	void __fastcall treeViewMenuClick(TObject *Sender);
	void __fastcall exitMenuClick(TObject *Sender);
	void __fastcall ListView2MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall lv2copyFileClick(TObject *Sender);
	void __fastcall lv2pasteFileClick(TObject *Sender);
	void __fastcall TreeView2MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall EUCKR1Click(TObject *Sender);
	void __fastcall UTF81Click(TObject *Sender);
	void __fastcall preViewButtonClick(TObject *Sender);
	void __fastcall preViewPanelResize(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall lv2allFileClick(TObject *Sender);

private:	// User declarations
	HWND hWnd;
	AnsiString rPath;
	int ImageCount;
	vector<int> ImageF;
	TStringList *fileSort;
	AnsiString PastF;
	vector<AnsiString> ExtImage;
	int mode;
	AnsiString listColumn1;
	AnsiString listColumn2;
	AnsiString listColumn3;
	AnsiString listColumn4;
	bool b_downBool;
	AnsiString s_PastDownPath;
public:		// User declarations
	AnsiString __fastcall func_dirFind();
	bool 	   __fastcall func_ftpListViewAdd(AnsiString Path);
	int  	   __fastcall func_ftpIconAdd(AnsiString Path);
	int  	   __fastcall func_ftpDirIconAdd();
	bool 	   __fastcall func_ftpTreeView(AnsiString Dir, TTreeNode *root);
	AnsiString __fastcall ftpGetAncestor(TTreeNode *node);
	void 	   __fastcall func_upLoadFile();
	void 	   __fastcall func_downLoad(AnsiString downPath);
	bool 	   __fastcall func_ftpConnState();
	void 	   __fastcall sort(int asc, int what);
	__fastcall TForm1(TComponent* Owner);
    int swsw;
  BEGIN_MESSAGE_MAP
  VCL_MESSAGE_HANDLER(0x0049,TWMDropFiles, WMDropFiles);
  VCL_MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,WMDropFiles)
  END_MESSAGE_MAP(TForm);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
